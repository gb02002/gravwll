#pragma once
#include "utils/namespaces/error_namespace.h"
#include <SDL3/SDL.h>
#include <memory>
#include <string>
#include <vulkan/vulkan_raii.hpp>

namespace window {
struct MyWindow {
  static constexpr const char *library = "SDL3";
  static constexpr const char *window_name = "Gravwll";
  std::unique_ptr<SDL_Window, decltype(&SDL_DestroyWindow)> instance;

  error::Result<bool> init(std::string name = window_name);

  MyWindow() : instance(nullptr, &SDL_DestroyWindow) {};
  MyWindow(MyWindow &&other) = default;
  MyWindow &operator=(MyWindow &&other) = default;

  MyWindow(const MyWindow &) = delete;
  MyWindow &operator=(const MyWindow &) = delete;

  error::CResult<vk::raii::SurfaceKHR>
  create_surface(vk::raii::Instance &vk_instance);
  error::Result<bool> show_window();

  int handle_events();
  // std::vector<const char *> get_extensions();
};
} // namespace window
