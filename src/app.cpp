#include "app.hpp"
#include "window.hpp"

#include <GLFW/glfw3.h>

#include <iostream>

namespace lvk {
void App::run() {
  create_window();
  main_loop();
}

void App::create_window() {
  m_window = glfw::create_window({1280, 720}, "Learn Vulkan");
}

void App::main_loop() {
  while (glfwWindowShouldClose(m_window.get()) != GLFW_FALSE)
    glfwPollEvents();
}
} // namespace lvk
