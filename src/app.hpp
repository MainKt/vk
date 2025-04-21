#pragma once

#include "gpu.hpp"
#include "window.hpp"

#include <vulkan/vulkan.hpp>

namespace lvk {
class App {
private:
  glfw::Window m_window{};
  vk::UniqueInstance m_instance{};
  vk::UniqueSurfaceKHR m_surface{};
  Gpu m_gpu{};

  void create_window();
  void main_loop();
  void create_instance();
  void create_surface();
	void select_gpu();

public:
  void run();
};
} // namespace lvk
