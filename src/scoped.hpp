#pragma once

#include <concepts>
#include <utility>

#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_handles.hpp>

namespace lvk {
template <typename Type>
concept Scopeable =
    std::equality_comparable<Type> && std::is_default_constructible_v<Type>;

template <Scopeable Type, typename Deleter> class Scoped {
public:
  Scoped(Scoped const &) = delete;
  auto operator=(Scoped const &) = delete;

  Scoped() = default;

  constexpr Scoped(Scoped &&rhs) noexcept
      : m_t{std::exchange(rhs.m_t, Type{})} {}

  constexpr Scoped &operator=(Scoped &&rhs) noexcept {
    if (&rhs != this)
      std::swap(m_t, rhs.m_t);
    return *this;
  }

  explicit(false) constexpr Scoped(Type t) : m_t{std::move(t)} {}

  constexpr ~Scoped() {
    if (m_t == Type{})
      return;
    Deleter{}(m_t);
  }

  [[nodiscard]] constexpr auto get() const -> Type const & { return m_t; }
  [[nodiscard]] constexpr auto get() -> Type & { return m_t; }

private:
  Type m_t{};
};

struct ScopedWaiterDeleter {
  void operator()(vk::Device const device) const noexcept { device.waitIdle(); }
};

using ScopedWaiter = Scoped<vk::Device, ScopedWaiterDeleter>;
} // namespace lvk
