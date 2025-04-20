#pragma once

#include <GLFW/glfw3.h>
#include <glm/common.hpp>
#include <glm/ext/vector_int2.hpp>

#include <memory>

namespace lvk::glfw {
struct Deleter {
  void operator()(GLFWwindow *window) const noexcept;
};

using Window = std::unique_ptr<GLFWwindow, lvk::glfw::Deleter>;

[[nodiscard]] Window create_window(glm::ivec2 size, char const *title);
} // namespace lvk::glfw
