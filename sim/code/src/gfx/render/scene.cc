#include "gfx/renderer/scene.h"
#include "utils/namespaces/error_namespace.h"

namespace gfx::renderer {
Scene::Scene() {
  camera_.set_perspective(60.0f, 16.0f / 9.0f, 0.1f, 1000.0f);
  camera_.look_at(glm::vec3(0.0f, 0.0f, 15.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                  glm::vec3(0.0f, 0.0f, 1.0f));
}

void Scene::set_particles(const std::vector<SceneParticle> &particles) {
  particles_ = particles;
}

void Scene::update(float delta_time) {
  debug::debug_print("Scene update called");
  // В будущем здесь может быть обновление частиц,
  // анимации, физика и т.д.
  // Сейчас оставляем пустым, так как камерой управляет CameraController
}
} // namespace gfx::renderer
