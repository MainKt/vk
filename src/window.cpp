#include "window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>
#include <stdexcept>

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
} // namespace lvk::glfw
