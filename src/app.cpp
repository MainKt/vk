#include "app.hpp"
#include "gpu.hpp"
#include "window.hpp"

#include <print>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>
#include <vulkan/vulkan_hpp_macros.hpp>
#include <vulkan/vulkan_structs.hpp>

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace lvk {
void App::run() {
  create_window();
  create_instance();
  create_surface();
  select_gpu();
  main_loop();
}

void App::create_window() {
  m_window = glfw::create_window({1280, 720}, "Learn Vulkan");
}

void App::main_loop() {
  while (glfwWindowShouldClose(m_window.get()) != GLFW_FALSE)
    glfwPollEvents();
}

void App::create_instance() {
  VULKAN_HPP_DEFAULT_DISPATCHER.init();
  auto const loader_version = vk::enumerateInstanceVersion();
  if (loader_version < vk_version_v)
    throw std::runtime_error{"loader does not support Vulkan 1.3"};

  vk::ApplicationInfo app_info;
  app_info.setPApplicationName("Learn Vulkan").setApiVersion(vk_version_v);

  vk::InstanceCreateInfo intance_ci;
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
} // namespace lvk
