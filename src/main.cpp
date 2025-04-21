#include "app.hpp"

#include <cstdlib>
#include <exception>
#include <print>

int main() {
  try {
    lvk::App{}.run();
  } catch (std::exception const &e) {
    std::println(stderr, "PANIC: {}", e.what());
    return EXIT_FAILURE;
  } catch (...) {
    std::println(stderr, "PANIC!");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
