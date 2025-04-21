#include "window.hpp"

#include <iostream>
#include <stdexcept>

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan_handles.hpp>

namespace lvk::glfw {
void Deleter::operator()(GLFWwindow *window) const noexcept {
  glfwDestroyWindow(window);
  glfwTerminate();
}

Window create_window(glm::ivec2 size, const char *title) {
  static auto const on_error = [](int const code, char const *desc) {
    std::println(std::cerr, "[GLFW] Error {}: {}", code, desc);
  };
  glfwSetErrorCallback(on_error);

  if (glfwInit() != GLFW_TRUE)
    throw std::runtime_error{"Vulkan not supported"};

  auto window = Window{};
  glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
  window.reset(glfwCreateWindow(size.x, size.y, title, nullptr, nullptr));

  if (!window)
    throw std::runtime_error{"Failed to create GLFW Window"};

  return window;
}

std::span<char const *const> instance_extensions() {
  std::uint32_t count{};
  auto const *extensions = glfwGetRequiredInstanceExtensions(&count);
  return {extensions, static_cast<std::size_t>(count)};
}
vk::UniqueSurfaceKHR create_surface(GLFWwindow *window,
                                    vk::Instance const instance) {
  VkSurfaceKHR surface;
  auto const result =
      glfwCreateWindowSurface(instance, window, nullptr, &surface);
  if (result != VK_SUCCESS || surface == VkSurfaceKHR{})
    throw std::runtime_error{"Failed to create Vulkan Surface"};

  return vk::UniqueSurfaceKHR{surface, instance};
}
} // namespace lvk::glfw
