#pragma once

#include "gpu.hpp"
#include "scoped.hpp"
#include "window.hpp"

#include <vulkan/vulkan.hpp>

namespace lvk {
class App {
public:
  void run();

private:
  void create_window();
  void main_loop();
  void create_instance();
  void create_surface();
  void select_gpu();
  void create_device();

  glfw::Window m_window{};
  vk::UniqueInstance m_instance{};
  vk::UniqueSurfaceKHR m_surface{};
  Gpu m_gpu{};
  vk::UniqueDevice m_device{};
  vk::Queue m_queue{};

  // waiter must remain at the end to be the first member that gets destroyed
  ScopedWaiter m_waiter{};
};
} // namespace lvk
