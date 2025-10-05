#pragma once
// #include "ctx/initial_condition_generator.h"
#include "ctx/simulation_config.h"
#include "ctx/simulation_state.h"
#include "ds/storage/storage.h"
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

  static DataCtx from_config(const SimulationConfig &config) {
    return DataCtx{.population_mode = config.data_population_mode,
                   .body_count = static_cast<size_t>(config.kNBodies),
                   .random_seed = config.random_seed};
  }
};

// Основной контекст - агрегирует специализированные контексты
class Ctx {
public:
  // Фабричный метод для создания контекста
  static std::unique_ptr<Ctx> create(SimulationConfig config) {
    return std::make_unique<Ctx>(std::move(config));
  }

  // Конструктор
  Ctx(SimulationConfig config);

  // Доступ к специализированным контекстам
  const GfxCtx &gfx() const { return gfx_ctx_; }
  PhysicsCtx &physics() { return physics_ctx_; }
  const DataCtx &data() const { return data_ctx_; }

  // Доступ к общему состоянию
  SimulationState &state() { return state_; }
  const SimulationState &state() const { return state_; }

  // Доступ к хранилищу
  Storage &storage() { return std::ref(storage_); }
  const Storage &storage() const { return storage_; }

  // Bounding box для пространственных операций
  const MyMath::BoundingBox &bounding_box() const { return bounding_box_; }

  // Обновление состояния
  void update_state(STATE new_state);

  // Валидация контекста
  bool validate() const;

private:
  void initialize_components();
  std::vector<Particle> create_initial_dataset();

  SimulationConfig config_;
  SimulationState state_;
  Storage storage_;

  // Специализированные контексты
  GfxCtx gfx_ctx_;
  PhysicsCtx physics_ctx_;
  DataCtx data_ctx_;

  // Пространственные параметры
  const MyMath::BoundingBox bounding_box_ = {{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}};
};

/* Это старый код, его надо перенести новые структуры
std::unique_ptr<Settings> settings;
const unsigned short TreeMaxDepth;
double TotalMass;
std::chrono::microseconds IntegrationStepInMicroseconds;

*/
