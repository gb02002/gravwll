#include "gfx/core/input.h"
#include "gfx/window.h"
#include <SDL3/SDL_events.h>
#include <SDL3/SDL_scancode.h>
#include <iostream>

namespace gfx::core {

InputManager::InputManager(window::MyWindow &window) : window_(window) {
  current_state_ = InputState();
  prev_state_ = InputState();

  for (int scancode = SDL_SCANCODE_UNKNOWN; scancode < SDL_SCANCODE_COUNT;
       ++scancode) {
    current_state_.keys[static_cast<SDL_Scancode>(scancode)] = KeyState{};
    prev_state_.keys[static_cast<SDL_Scancode>(scancode)] = KeyState{};
  }
}

InputManager::~InputManager() {
  if (mouse_captured_)
    set_mouse_capture(false);
}

void InputManager::precess_events() {
  prev_state_.mouse = current_state_.mouse;
  current_state_.quit_requested = false;
  current_state_.mouse.delta = glm::vec2(0.0f);
  current_state_.mouse.wheel = 0.0f;

  for (auto &[scancode, key_state] : current_state_.keys) {
    key_state.pressed = false;
    key_state.released = false;
  }

  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    handle_sdl_event_(event);
  }

  if (!mouse_captured_) {
    current_state_.mouse.delta =
        current_state_.mouse.mouse_position - prev_state_.mouse.mouse_position;
  }

  for (auto &[scancode, current_key] : current_state_.keys) {
    auto prev_it = prev_state_.keys.find(scancode);
    if (prev_it != prev_state_.keys.end()) {
      const KeyState &previous_key = prev_it->second;

      if (current_key.held && !previous_key.held) {
        current_key.pressed = true;
      }

      if (!current_key.held && previous_key.held) {
        current_key.released = true;
      }
    } else {
      if (current_key.held) {
        current_key.pressed = true;
      }
    }
  }

  prev_state_.keys = current_state_.keys;
}

void InputManager::handle_sdl_event_(const SDL_Event &event) {
  switch (event.type) {
  case SDL_EVENT_QUIT:
    current_state_.quit_requested = true;
    break;

  case SDL_EVENT_KEY_DOWN:
    update_key_state_(event.key.scancode, true);
    break;

  case SDL_EVENT_KEY_UP:
    update_key_state_(event.key.scancode, false);
    break;

  case SDL_EVENT_MOUSE_MOTION:
    if (mouse_captured_) {
      current_state_.mouse.delta.x += event.motion.xrel;
      current_state_.mouse.delta.y += event.motion.yrel;

      current_state_.mouse.mouse_position.x += event.motion.xrel;
      current_state_.mouse.mouse_position.y += event.motion.yrel;
    } else {
      current_state_.mouse.mouse_position.x = event.motion.x;
      current_state_.mouse.mouse_position.y = event.motion.y;
    }
    break;

  case SDL_EVENT_MOUSE_BUTTON_DOWN:
  case SDL_EVENT_MOUSE_BUTTON_UP: {
    bool pressed = (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN);

    switch (event.button.button) {
    case SDL_BUTTON_LEFT:
      current_state_.mouse.lmb = pressed;
      break;
    case SDL_BUTTON_RIGHT:
      current_state_.mouse.rmb = pressed;
      break;
    case SDL_BUTTON_MIDDLE:
      current_state_.mouse.mmb = pressed;
      break;
    }
    break;
  }

  case SDL_EVENT_MOUSE_WHEEL:
    current_state_.mouse.delta += event.wheel.y;
    break;

  case SDL_EVENT_WINDOW_FOCUS_LOST:
    set_mouse_capture(false);
    break;
  }
}

void InputManager::update_key_state_(SDL_Scancode scancode, bool pressed) {
  auto it = current_state_.keys.find(scancode);
  if (it != current_state_.keys.end()) {
    it->second.held = pressed;
  }
}

void InputManager::set_mouse_capture(bool capture) {
  if (capture == mouse_captured_) {
    return;
  }

  mouse_captured_ = capture;

  if (capture) {
    SDL_CaptureMouse(true);
    SDL_SetWindowRelativeMouseMode(window_.instance.get(), true);

    current_state_.mouse.delta = glm::vec2(0.0f);
  } else {
    SDL_CaptureMouse(false);
    SDL_SetWindowRelativeMouseMode(window_.instance.get(), false);
  }

  std::cout << "Mouse capture " << (capture ? "enabled" : "disabled")
            << std::endl;
}
} // namespace gfx::core
