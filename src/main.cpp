#include "app.hpp"

#include <cstdlib>
#include <exception>
#include <iostream>
#include <ostream>

int main() {
  try {
    lvk::App{}.run();
  } catch (std::exception const &e) {
    std::println(std::cerr, "PANIC: {}", e.what());
    return EXIT_FAILURE;
  } catch (...) {
    std::println(std::cerr, "PANIC!");
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
