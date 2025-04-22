#pragma once

#include "gpu.hpp"
#include "render_target.hpp"

#include <glm/ext/vector_int2.hpp>
#include <vulkan/vulkan.hpp>

namespace lvk {
class Swapchain {
public:
  explicit Swapchain(vk::Device device, Gpu const &gpu, vk::SurfaceKHR surface,
                     glm::ivec2 size);

  bool recreate(glm::ivec2 size);

  [[nodiscard]] glm::ivec2 get_size() const {
    return {m_ci.imageExtent.width, m_ci.imageExtent.height};
  }

private:
  void populate_images();
  void create_image_views();
  std::optional<RenderTarget> acquire_next_image(vk::Semaphore const to_signal);
  bool present(vk::Queue const q, vk::Semaphore const to_wait);
	vk::ImageMemoryBarrier2 base_barrier() const;

  vk::Device m_device{};
  Gpu m_gpu;

  vk::SwapchainCreateInfoKHR m_ci{};
  vk::UniqueSwapchainKHR m_swapchain{};
  std::vector<vk::Image> m_images{};
  std::vector<vk::UniqueImageView> m_image_views{};
  std::optional<std::size_t> m_image_index{};
};

bool needs_recreation(vk::Result const result);
} // namespace lvk
