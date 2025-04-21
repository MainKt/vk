#pragma once

#include <memory>
#include <span>

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/ext/vector_int2.hpp>
#include <vulkan/vulkan.hpp>

namespace lvk::glfw {
struct Deleter {
  void operator()(GLFWwindow *window) const noexcept;
};

using Window = std::unique_ptr<GLFWwindow, lvk::glfw::Deleter>;

[[nodiscard]] Window create_window(glm::ivec2 size, char const *title);

[[nodiscard]] std::span<char const *const> instance_extensions();
[[nodiscard]] vk::UniqueSurfaceKHR create_surface(GLFWwindow *window,
                                                  vk::Instance const instance);
} // namespace lvk::glfw
