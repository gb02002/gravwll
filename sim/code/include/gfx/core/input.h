#pragma once
#include "SDL3/SDL_scancode.h"
#include "gfx/window.h"
#include <SDL3/SDL_events.h>
#include <glm/ext/vector_float2.hpp>
#include <unordered_map>

namespace gfx::core {
struct KeyState {
  bool pressed, released, held;
};

struct MouseState {
  glm::vec2 mouse_position = glm::vec2(0.0f);
  glm::vec2 delta = glm::vec2(1.0f);
  float wheel = 0.0f;
  bool lmb;
  bool rmb;
  bool mmb;
};

struct InputState {
  std::unordered_map<SDL_Scancode, KeyState> keys;
  MouseState mouse;
  bool quit_requested;
};

class InputManager {
public:
  InputManager(window::MyWindow &window);
  ~InputManager();

  void precess_events();

  const InputState &get_state() const { return current_state_; };

  void set_mouse_capture(bool capture);
  bool is_mouse_captured() const { return mouse_captured_; };

private:
  void handle_sdl_event_(const SDL_Event &event);
  void update_key_state_(SDL_Scancode scancode, bool pressed);

  InputState current_state_;
  InputState prev_state_;
  bool mouse_captured_ = false;
  window::MyWindow &window_;
};

inline bool is_key_pressed(const InputState &state, SDL_Scancode key) {
  auto it = state.keys.find(key);
  return (it != state.keys.end()) && it->second.pressed;
}

inline bool is_key_released(const InputState &state, SDL_Scancode key) {
  auto it = state.keys.find(key);
  return (it != state.keys.end()) && it->second.released;
}

inline bool is_key_held(const InputState &state, SDL_Scancode key) {
  auto it = state.keys.find(key);
  return (it != state.keys.end()) && it->second.held;
}

namespace input_keys {
constexpr SDL_Scancode FORWARD = SDL_SCANCODE_W;
constexpr SDL_Scancode BACKWARD = SDL_SCANCODE_S;
constexpr SDL_Scancode LEFT = SDL_SCANCODE_A;
constexpr SDL_Scancode RIGHT = SDL_SCANCODE_D;
constexpr SDL_Scancode UP = SDL_SCANCODE_E;
constexpr SDL_Scancode DOWN = SDL_SCANCODE_Q;
constexpr SDL_Scancode SPRINT = SDL_SCANCODE_LSHIFT;
constexpr SDL_Scancode RESET_CAMERA = SDL_SCANCODE_R;
constexpr SDL_Scancode TOGGLE_MOUSE = SDL_SCANCODE_F1;
constexpr SDL_Scancode QUIT = SDL_SCANCODE_ESCAPE;
} // namespace input_keys
} // namespace gfx::core
