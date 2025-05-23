#include "app.hpp"
#include "gpu.hpp"
#include "window.hpp"

#include <chrono>
#include <print>
#include <ranges>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>

using namespace std::chrono_literals;

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace lvk {
void App::run() {
  create_window();
  create_instance();
  create_surface();
  select_gpu();
  create_device();
  create_swapchain();
  create_render_sync();
  main_loop();
}

void App::create_window() {
  m_window = glfw::create_window({1280, 720}, "Learn Vulkan");
}

void App::main_loop() {
  while (glfwWindowShouldClose(m_window.get()) == GLFW_FALSE) {
    glfwPollEvents();

    if (!acquire_render_target())
      continue;

    auto const command_buffer = begin_frame();
    transition_for_render(command_buffer);
    render(command_buffer);
    transition_for_render(command_buffer);
    submit_and_present();
  }
}

void App::create_instance() {
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
  auto const loader_version = vk::enumerateInstanceVersion();
  if (loader_version < vk_version_v)
    throw std::runtime_error{"loader does not support Vulkan 1.3"};

  vk::ApplicationInfo app_info{};
  app_info.setPApplicationName("Learn Vulkan").setApiVersion(vk_version_v);

  vk::InstanceCreateInfo intance_ci{};
  auto const extensions = glfw::instance_extensions();
  intance_ci.setPApplicationInfo(&app_info).setPEnabledExtensionNames(
      extensions);

  m_instance = vk::createInstanceUnique(intance_ci);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_instance);
}

void App::create_surface() {
  m_surface = glfw::create_surface(m_window.get(), *m_instance);
}

void App::select_gpu() {
  m_gpu = get_suitable_gpu(*m_instance, *m_surface);
  std::println("Using GPU: {}", std::string_view{m_gpu.properties.deviceName});
}

void App::create_device() {
  vk::DeviceQueueCreateInfo queue_ci{};
  static constexpr std::array queue_priorities_v{1.0f};
  queue_ci.setQueueFamilyIndex(m_gpu.queue_family)
      .setQueueCount(1)
      .setQueuePriorities(queue_priorities_v);

  vk::PhysicalDeviceFeatures enabled_features{};
  enabled_features.fillModeNonSolid = m_gpu.features.fillModeNonSolid;
  enabled_features.wideLines = m_gpu.features.wideLines;
  enabled_features.samplerAnisotropy = m_gpu.features.samplerAnisotropy;
  enabled_features.sampleRateShading = m_gpu.features.sampleRateShading;

  vk::PhysicalDeviceSynchronization2Features sync_feature{vk::True};
  vk::PhysicalDeviceDynamicRenderingFeatures dynamic_rendering_feature{
      vk::True};
  sync_feature.setPNext(&dynamic_rendering_feature);

  vk::DeviceCreateInfo device_ci{};
  static constexpr std::array extensions_v = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};
  device_ci.setPEnabledExtensionNames(extensions_v)
      .setQueueCreateInfos(queue_ci)
      .setPEnabledFeatures(&enabled_features)
      .setPNext(&sync_feature);

  m_device = m_gpu.device.createDeviceUnique(device_ci);
  VULKAN_HPP_DEFAULT_DISPATCHER.init(*m_device);

  static constexpr std::uint32_t queue_index_v{0};
  m_queue = m_device->getQueue(m_gpu.queue_family, queue_index_v);

  m_waiter = *m_device;
}

void App::create_swapchain() {
  auto const size = glfw::framebuffer_size(m_window.get());
  m_swapchain.emplace(*m_device, m_gpu, *m_surface, size);
}

void App::create_render_sync() {
  vk::CommandPoolCreateInfo command_pool_ci{};
  command_pool_ci.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer)
      .setQueueFamilyIndex(m_gpu.queue_family);
  m_render_cmd_pool = m_device->createCommandPoolUnique(command_pool_ci);

  vk::CommandBufferAllocateInfo command_buffer_ai{};
  command_buffer_ai.setCommandPool(*m_render_cmd_pool)
      .setCommandBufferCount(static_cast<std::uint32_t>(resource_buffering_v))
      .setLevel(vk::CommandBufferLevel::ePrimary);

  auto const command_buffers =
      m_device->allocateCommandBuffers(command_buffer_ai);
  assert(command_buffers.size() == m_render_sync.size());

  static constexpr vk::FenceCreateInfo fence_create_info_v{
      vk::FenceCreateFlagBits::eSignaled};
  for (auto [sync, command_buffer] :
       std::views::zip(m_render_sync, command_buffers)) {
    sync.command_buffer = command_buffer;
    sync.draw = m_device->createSemaphoreUnique({});
    sync.present = m_device->createSemaphoreUnique({});
    sync.drawn = m_device->createFenceUnique(fence_create_info_v);
  }
}

bool App::acquire_render_target() {
  m_framebuffer_size = glfw::framebuffer_size(m_window.get());

  if (m_framebuffer_size.x <= 0 || m_framebuffer_size.y <= 0)
    return false;

  auto &render_sync = m_render_sync.at(m_frame_index);

  static constexpr auto fence_timeout_v =
      static_cast<std::uint64_t>(std::chrono::nanoseconds{3s}.count());
  auto result =
      m_device->waitForFences(*render_sync.drawn, vk::True, fence_timeout_v);
  if (result != vk::Result::eSuccess)
    throw std::runtime_error{"Failed to wait for Render Fence"};

  m_render_target = m_swapchain->acquire_next_image(*render_sync.draw);
  if (!m_render_target) {
    m_swapchain->recreate(m_framebuffer_size);
    return false;
  }

  m_device->resetFences(*render_sync.drawn);
  return true;
}

vk::CommandBuffer App::begin_frame() {
  auto const &render_sync = m_render_sync.at(m_frame_index);

  vk::CommandBufferBeginInfo command_buffer_bi{};
  command_buffer_bi.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);
  render_sync.command_buffer.begin(command_buffer_bi);
  return render_sync.command_buffer;
}

void App::transition_for_render(vk::CommandBuffer command_buffer) const {
  vk::DependencyInfo dependency_info{};
  auto barrier = m_swapchain->base_barrier();
  barrier.setOldLayout(vk::ImageLayout::eUndefined)
      .setNewLayout(vk::ImageLayout::eAttachmentOptimal)
      .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentRead |
                        vk::AccessFlagBits2::eColorAttachmentWrite)
      .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
      .setDstAccessMask(barrier.srcAccessMask)
      .setDstStageMask(barrier.srcStageMask);
  dependency_info.setImageMemoryBarriers(barrier);
  command_buffer.pipelineBarrier2(dependency_info);
}

void App::render(vk::CommandBuffer command_buffer) {
  vk::RenderingAttachmentInfo color_attachment{};
  color_attachment.setImageView(m_render_target->image_view)
      .setImageLayout(vk::ImageLayout::eAttachmentOptimal)
      .setLoadOp(vk::AttachmentLoadOp::eClear)
      .setStoreOp(vk::AttachmentStoreOp::eStore)
      .setClearValue(vk::ClearColorValue{1.0f, 0.0f, 0.0f, 1.0f});

  vk::RenderingInfo rendering_info{};
  vk::Rect2D render_area{vk::Offset2D{}, m_render_target->extent};
  rendering_info.setRenderArea(render_area)
      .setColorAttachments(color_attachment)
      .setLayerCount(1);

  command_buffer.beginRendering(rendering_info);
  // draw stuff
  command_buffer.endRendering();
}

void App::transition_for_present(vk::CommandBuffer command_buffer) const {
  vk::DependencyInfo dependency_info{};
  auto barrier = m_swapchain->base_barrier();
  barrier.setOldLayout(vk::ImageLayout::eAttachmentOptimal)
      .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
      .setSrcAccessMask(vk::AccessFlagBits2::eColorAttachmentRead |
                        vk::AccessFlagBits2::eColorAttachmentWrite)
      .setSrcStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput)
      .setDstAccessMask(barrier.srcAccessMask)
      .setDstStageMask(barrier.srcStageMask);
  dependency_info.setImageMemoryBarriers(barrier);
  command_buffer.pipelineBarrier2(dependency_info);
}

void App::submit_and_present() {
  auto const &render_sync = m_render_sync.at(m_frame_index);
  render_sync.command_buffer.end();

  vk::CommandBufferSubmitInfo command_buffer_info{render_sync.command_buffer};

  vk::SemaphoreSubmitInfo wait_semaphore_info{};
  wait_semaphore_info.setSemaphore(*render_sync.draw)
      .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::SemaphoreSubmitInfo signal_semaphore_info{};
  signal_semaphore_info.setSemaphore(*render_sync.present)
      .setStageMask(vk::PipelineStageFlagBits2::eColorAttachmentOutput);

  vk::SubmitInfo2 submit_info{};
  submit_info.setCommandBufferInfos(command_buffer_info)
      .setWaitSemaphoreInfos(wait_semaphore_info)
      .setSignalSemaphoreInfos(signal_semaphore_info);
  m_queue.submit2(submit_info, *render_sync.drawn);

  m_frame_index = (m_frame_index + 1) % m_render_sync.size();
  m_render_target.reset();

  auto const fb_size_changed = m_framebuffer_size != m_swapchain->get_size();
  auto const out_of_date = !m_swapchain->present(m_queue, *render_sync.present);
  if (fb_size_changed || out_of_date)
    m_swapchain->recreate(m_framebuffer_size);
}
} // namespace lvk
