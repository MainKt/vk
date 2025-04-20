#pragma once

#include "window.hpp"

namespace lvk {
class App {
private:
  glfw::Window m_window{};

  void create_window();
  void main_loop();

public:
  void run();
};
} // namespace lvk
