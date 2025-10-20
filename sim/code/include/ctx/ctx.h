#pragma once
#include "ds/storage/storage.h"
#include "simulation_config.h"
#include "simulation_state.h"
#include "utils/namespaces/MyMath.h"
#include <chrono>
#include <functional>
#include <memory>
#include <sys/types.h>
#include <vector>

// Контекст только для графики
struct GfxCtx {
  ushort desired_fps = 60;
  bool headless = false;
  bool verbose = false;

  // Только то, что нужно графике
  static GfxCtx from_config(const SimulationConfig &config) {
    return GfxCtx{.desired_fps = config.kFpsDesired,
                  .headless = config.kHeadless,
                  .verbose = config.kVerbose};
  }
};

// Контекст только для физики
struct PhysicsCtx {
  std::chrono::microseconds integration_step;
  unsigned short tree_max_depth = 10;
  bool debug = false;

  static PhysicsCtx from_config(const SimulationConfig &config) {
    return PhysicsCtx{.integration_step =
                          std::chrono::microseconds(config.integration_step),
                      .tree_max_depth = config.kTreeMaxDepth,
                      .debug = config.kDebug};
  }
  const unsigned short tree_depth() const { return tree_max_depth; }
};

// Контекст для данных
struct DataCtx {
  std::vector<Particle> initial_dataset;
  SimulationConfig::PUPULATION_MODE population_mode;
  size_t body_count = 0;
  int random_seed = 42;
  const MyMath::BoundingBox bounding_box_ = {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}};

  static DataCtx from_config(const SimulationConfig &config) {
    return DataCtx{.population_mode = config.data_population_mode,
                   .body_count = static_cast<size_t>(config.kNBodies),
                   .random_seed = config.random_seed};
  }
  const std::vector<Particle> access_dataset() { return initial_dataset; }
};

// Основной контекст - агрегирует специализированные контексты
class Ctx {
public:
  static std::unique_ptr<Ctx> create(SimulationConfig config) {
    return std::make_unique<Ctx>(std::move(config));
  }

  Ctx(SimulationConfig config);

  GfxCtx &gfx() { return gfx_ctx_; }
  PhysicsCtx &physics() { return physics_ctx_; }
  DataCtx &data() { return data_ctx_; }

  SimulationState &state() { return state_; }
  const SimulationState &state() const { return state_; }

  Storage &storage() { return std::ref(storage_); }
  const Storage &storage() const { return storage_; }

  const MyMath::BoundingBox &bounding_box() const {
    return data_ctx_.bounding_box_;
  }

  void update_state(STATE new_state);

  bool validate() const;

private:
  void initialize_components();
  std::vector<Particle> create_initial_dataset();

  SimulationConfig config_;
  SimulationState state_;
  Storage storage_;

  GfxCtx gfx_ctx_;
  PhysicsCtx physics_ctx_;
  DataCtx data_ctx_;
};
