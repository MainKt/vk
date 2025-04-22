#pragma once

#include "gpu.hpp"
#include "resource_buffering.hpp"
#include "scoped.hpp"
#include "swapchain.hpp"
#include "window.hpp"

#include <optional>

#include <vulkan/vulkan.hpp>

namespace lvk {
class App {
public:
  void run();

private:
  struct RenderSync {
    vk::UniqueSemaphore draw{};
    vk::UniqueSemaphore present{};
    vk::UniqueFence drawn{};
    vk::CommandBuffer command_buffer{};
  };

  void create_window();
  void main_loop();
  void create_instance();
  void create_surface();
  void select_gpu();
  void create_device();
  void create_swapchain();
	void create_render_sync();

  glfw::Window m_window{};
  vk::UniqueInstance m_instance{};
  vk::UniqueSurfaceKHR m_surface{};
  Gpu m_gpu{};
  vk::UniqueDevice m_device{};
  std::optional<Swapchain> m_swapchain{};
  vk::Queue m_queue{};

	vk::UniqueCommandPool m_render_cmd_pool{};
	Buffered<RenderSync> m_render_sync{};
	std::size_t m_frame_inex{};

  // waiter must remain at the end to be the first member that gets destroyed
  ScopedWaiter m_waiter{};
};
} // namespace lvk
