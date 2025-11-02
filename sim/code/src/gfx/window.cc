#include "gfx/window.h"
#include "SDL3/SDL_vulkan.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_error.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_hints.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_video.h"
#include "gravwll/third_party/sdl3/include/SDL3/SDL_vulkan.h"
#include "gravwll/third_party/sdl3/src/video/khronos/vulkan/vulkan_core.h"
#include "utils/namespaces/error_namespace.h"
#include <SDL3/SDL_hints.h>
#include <SDL3/SDL_scancode.h>
#include <string>
#include <utility>
#include <vulkan/vulkan_raii.hpp>

#define WIDTH 800
#define HEIGHT 600

namespace window {

error::Result<bool> MyWindow::init(std::string windown_name) {
  debug::debug_print("Инициализация SDL3... Из нового места");

  SDL_SetHint(SDL_HINT_VIDEO_DRIVER, "wayland");
  int num_drivers = SDL_GetNumVideoDrivers();
  debug::debug_print("Доступно видео драйверов: {}", num_drivers);

  for (int i = 0; i < num_drivers; i++) {
    debug::debug_print("  Драйвер {}: {}", i, SDL_GetVideoDriver(i));
  }

  bool sdl_window_result = SDL_Init(SDL_INIT_VIDEO);
  debug::debug_print("sdl_window_result: {}", sdl_window_result);
  if (!sdl_window_result)
    return error::Result<bool>::error(1, "Failed to initialize SDL");

  if (!SDL_Vulkan_LoadLibrary(nullptr))
    return error::Result<bool>::error(1, "Failed to load SDL::Vulkan");

  SDL_Window *window = SDL_CreateWindow(
      windown_name.c_str(), WIDTH, HEIGHT,
      SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_HIDDEN);
  if (!window)
    return error::Result<bool>::error(1, SDL_GetError());

  debug::debug_print("Текущий видео драйвер: {}", SDL_GetCurrentVideoDriver());
  instance.reset(window);
  SDL_MaximizeWindow(window);
  return error::Result<bool>::success(true);
};

error::CResult<vk::raii::SurfaceKHR>
MyWindow::create_surface(vk::raii::Instance &vk_instance) {
  VkSurfaceKHR raw_surface_ = VK_NULL_HANDLE;
  bool res = SDL_Vulkan_CreateSurface(instance.get(), *vk_instance, nullptr,
                                      &raw_surface_);
  if (!res) {
    debug::debug_print("Failed to create surface: {}", SDL_GetError());
    return error::CResult<vk::raii::SurfaceKHR>::error(1,
                                                       "Surface init failure");
  }
  vk::raii::SurfaceKHR surface{vk_instance, raw_surface_, nullptr};
  // std::pair<typename T1, typename T2>
  return error::CResult<vk::raii::SurfaceKHR>::success(std::move(surface));
}

error::Result<bool> window::MyWindow::show_window() {
  bool success = SDL_ShowWindow(instance.get());
  if (!success) {
    const char *err = SDL_GetError();
    return error::Result<bool>::error(1, err);
  }
  return error::Result<bool>::success(success);
}

int MyWindow::handle_events() {
  for (SDL_Event event; SDL_PollEvent(&event);)
    switch (event.key.scancode) {
    case SDL_SCANCODE_ESCAPE: {
      std::cout << "We got SDL_EVENT_QUIT" << std::endl;
      return 1;
    }
    default:
      break;
    }
  return 0;
}

} // namespace window
