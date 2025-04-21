#pragma once

#include "window.hpp"

#include <vulkan/vulkan.hpp>

namespace lvk {
class App {
private:
  glfw::Window m_window{};
  vk::UniqueInstance m_instance{};
  vk::UniqueSurfaceKHR m_surface{};

  void create_window();
  void main_loop();
  void create_instance();
	void create_surface();

public:
  void run();
};
} // namespace lvk
