#include "swapchain.hpp"

#include <algorithm>
#include <print>
#include <stdexcept>

#include <glm/vec2.hpp>

namespace lvk {
constexpr std::array srgb_formats_v{
    vk::Format::eR8G8B8A8Srgb,
    vk::Format::eB8G8R8A8Srgb,
};

constexpr std::uint32_t min_images_v{3};

[[nodiscard]] vk::SurfaceFormatKHR
get_surface_format(std::span<vk::SurfaceFormatKHR const> supported) {
  for (auto const desired : srgb_formats_v) {
    auto const is_match = [desired](vk::SurfaceFormatKHR const &in) {
      return in.format == desired &&
             in.colorSpace == vk::ColorSpaceKHR::eVkColorspaceSrgbNonlinear;
    };
    auto const it = std::ranges::find_if(supported, is_match);
    if (it == supported.end())
      continue;
    return *it;
  }
  return supported.front();
}

[[nodiscard]] constexpr vk::Extent2D
get_image_extent(vk::SurfaceCapabilitiesKHR const &caps,
                 glm::uvec2 const size) {
  constexpr auto limitless_v = 0xffffffff;
  if (caps.currentExtent.width < limitless_v &&
      caps.currentExtent.height < limitless_v)
    return caps.currentExtent;

  auto const x =
      std::clamp(size.x, caps.minImageExtent.width, caps.maxImageExtent.width);
  auto const y = std::clamp(size.y, caps.minImageExtent.height,
                            caps.maxImageExtent.height);

  return vk::Extent2D{x, y};
}

[[nodiscard]] constexpr std::uint32_t
get_image_count(vk::SurfaceCapabilitiesKHR const &caps) {
  if (caps.maxImageCount < caps.minImageCount)
    return std::max(min_images_v, caps.minImageCount);
  return std::clamp(min_images_v, caps.minImageCount, caps.maxImageCount);
}

void require_success(vk::Result const result, char const *error_msg) {
  if (result != vk::Result::eSuccess)
    throw std::runtime_error{error_msg};
}

Swapchain::Swapchain(vk::Device device, Gpu const &gpu, vk::SurfaceKHR surface,
                     glm::ivec2 size)
    : m_device{device}, m_gpu{gpu} {
  auto const surface_format =
      get_surface_format(m_gpu.device.getSurfaceFormatsKHR(surface));

  m_ci.setSurface(surface)
      .setImageFormat(surface_format.format)
      .setImageColorSpace(surface_format.colorSpace)
      .setImageArrayLayers(1)
      .setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
      .setPresentMode(vk::PresentModeKHR::eFifo);

  if (!recreate(size))
    throw std::runtime_error{"Failed to create Vulkan Swapchain"};
}

bool Swapchain::recreate(glm::ivec2 size) {
  if (size.x <= 0 || size.y <= 0)
    return false;

  auto const caps = m_gpu.device.getSurfaceCapabilitiesKHR(m_ci.surface);
  m_ci.setImageExtent(get_image_extent(caps, size))
      .setMinImageCount(get_image_count(caps))
      .setOldSwapchain(m_swapchain ? *m_swapchain : vk::SwapchainKHR{})
      .setQueueFamilyIndices(m_gpu.queue_family);

  assert(m_ci.imageExtent.width > 0 && m_ci.imageExtent.height > 0 &&
         m_ci.minImageCount >= min_images_v);

  m_device.waitIdle();
  m_swapchain = m_device.createSwapchainKHRUnique(m_ci);

  populate_images();
  create_image_views();

  size = get_size();
  std::println("[lvk] Swapchain [{}x{}]", size.x, size.y);

  return true;
}

void Swapchain::populate_images() {
  std::uint32_t image_count{};
  auto result =
      m_device.getSwapchainImagesKHR(*m_swapchain, &image_count, nullptr);
  require_success(result, "Failed to get Swapchain Images");

  m_images.resize(image_count);
  result = m_device.getSwapchainImagesKHR(*m_swapchain, &image_count,
                                          m_images.data());
  require_success(result, "Failed to get Swapchain Images");
}

void Swapchain::create_image_views() {
  vk::ImageSubresourceRange subresource_range{};
  subresource_range.setAspectMask(vk::ImageAspectFlagBits::eColor)
      .setLayerCount(1)
      .setLevelCount(1);

  vk::ImageViewCreateInfo image_view_ci{};
  image_view_ci.setViewType(vk::ImageViewType::e2D)
      .setFormat(m_ci.imageFormat)
      .setSubresourceRange(subresource_range);

  m_image_views.clear();
  m_image_views.reserve(m_images.size());

  for (auto const image : m_images) {
    image_view_ci.setImage(image);
    m_image_views.push_back(m_device.createImageViewUnique(image_view_ci));
  }
}
} // namespace lvk
