#pragma once

#include "../core/input.h"
#include "utils/namespaces/error_namespace.h"

namespace gfx::renderer {

class BaseRenderer {
public:
  virtual ~BaseRenderer() = default;

  virtual error::Result<bool> init() = 0;
  virtual error::Result<bool> render_frame() = 0;
  virtual error::Result<bool> update(float delta_time) = 0;

  virtual core::InputManager &get_input_manager() = 0;
};
} // namespace gfx::renderer
