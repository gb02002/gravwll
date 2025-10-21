#define CURRENT_MODULE_DEBUG 1

#include "ctx/ctx.h"
#include "ctx/dataSets.h"
#include "ctx/simulation_config.h"
#include "ctx/simulation_state.h"
#include "ds/storage/storage.h"
#include "utils/generators.h"
#include <exception>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

using namespace error;

Ctx::Ctx(SimulationConfig config)
    : config_(std::move(config)), storage_(config_.kNBodies),
      gfx_ctx_(GfxCtx::from_config(config_)),
      physics_ctx_(PhysicsCtx::from_config(config_)),
      data_ctx_(DataCtx::from_config(config_)) {
  initialize_components();
}

void Ctx::initialize_components() {
  state_.set_state(STATE::INITIALIZING);

  try {
    // 1. сначала генератор
    data_ctx_.initial_dataset = create_initial_dataset();
    debug::debug_print("CREATE_INITIAL_DATASET: len: {}",
                       data_ctx_.access_dataset().size());

    if (!validate()) {
      throw std::runtime_error("Context validation failed!");
    }
    state_.set_state(STATE::WARM_UP);
  } catch (const std::exception &e) {
    state_.set_state(STATE::ERROR);
    if (config_.kVerbose) {
      std::cerr << "Initialization error: " << e.what() << std::endl;
    }
    throw;
  }
  /*
  2. потом сторадж(получаем примерный layout -> тянем арену для всего ->
  формируем примерный layout для регистрации блоков)
  3. формируем стейт из общего и важного(инициализуем структуры для компонентов
  + общий state)
  4. Сплитим отдельно графический и физичксий конфиг
  5. После наполняем нужный остаточный кoнфиг(что-то вспомогательное, типо
  логов, флагов дебага). Скорее всего надо переназвать SimulationConfig в
  WarmUpConfig, а SimulationConfig сделать внутренним
  */
}

std::vector<Particle> Ctx::create_initial_dataset() {
  switch (config_.data_population_mode) {
  case SimulationConfig::PUPULATION_MODE::PLUMMER: {
    CResult<std::vector<Particle>> res = generators::generate_plummer(
        data_ctx_.body_count, data_ctx_.bounding_box_, data_ctx_.random_seed,
        generator_structs::PlummerParams{});
    if (res.is_ok()) {
      return res.unwrap();
    } else {
      debug::debug_print("PLUMMER failed: msg: {}", res.error_message());
      return {};
    }
  }

  case SimulationConfig::PUPULATION_MODE::UNIFORM: {
    CResult<std::vector<Particle>> res = generators::generate_uniform(
        bounding_box(), data_ctx_.body_count, data_ctx_.random_seed);
    if (res.is_ok()) {
      return res.unwrap();
    } else {
      debug::debug_print("UNIFORM failed: msg: {}", res.error_message());
      return {};
    }
  }
  case SimulationConfig::PUPULATION_MODE::FILE: {
    CResult<std::vector<Particle>> res =
        data_loader::load_from_file(config_.filename);
    if (res.is_ok()) {
      return res.unwrap();
    } else {
      debug::debug_print("generate_uniform failed: msg: {}",
                         res.error_message());
      return {};
    }
  }
  case SimulationConfig::PUPULATION_MODE::FETCH: {
    if (config_.data_set_name.empty()) {
      throw std::runtime_error("Dataset name not specified for FETCH mode");
    }
    CResult<std::vector<Particle>> res = data_loader::download_dataset(
        config_.data_set_name, data_ctx_.body_count);
    if (res.is_ok()) {
      return res.unwrap();
    } else {
      debug::debug_print("generate_uniform failed: msg: {}",
                         res.error_message());
      return {};
    }
  }
  case SimulationConfig::PUPULATION_MODE::EMPTY:
    return generators::generate_empty();

  case SimulationConfig::PUPULATION_MODE::KeplerianDisk: {
    CResult<std::vector<Particle>> res = generators::generate_keplerian_disk(
        data_ctx_.body_count, data_ctx_.random_seed);
    if (res.is_ok()) {
      return res.unwrap();
    } else {
      debug::debug_print("generate_uniform failed: msg: {}",
                         res.error_message());
      return {};
    }
  }
  default:
    throw std::runtime_error("Unknown population mode");
  }
}
void Ctx::update_state(STATE new_state) { state_.set_state(new_state); }

bool Ctx::validate() const {
  if (physics_ctx_.integration_step.count() <= 0) {
    if (config_.kVerbose) {
      std::cerr << "Invalid integration_step: "
                << physics_ctx_.integration_step.count() << std::endl;
    }
    return false;
  }
  if (data_ctx_.body_count == 0 &&
      data_ctx_.population_mode != SimulationConfig::PUPULATION_MODE::EMPTY) {
    if (config_.kVerbose) {
      std::cerr << "Invalid body count: " << data_ctx_.body_count << std::endl;
    }
    return false;
  }

  if (gfx_ctx_.desired_fps == 0 && !gfx_ctx_.headless) {
    if (config_.kVerbose) {
      std::cerr << "Invalid FPS: " << gfx_ctx_.desired_fps << std::endl;
    }
    return false;
  }

  return true;
}

// void Cfx::adoptInitialValues() {
//   std::cout << "In this function we need to retrive data from settings and
//   "
//                "prepare them for cfx\n";
//
//   InitialBB = {{0, 0, 0}, {1, 1, 1}};
//   IntegrationStepInMicroseconds =
//       std::chrono::microseconds(settings->IntegrationStep);
// }
//
// // Plammer to be implemented later
// std::vector<Particle> Cfx::CreateDataSet() {
//   auto dataSet = GenerateRandomParticle(this->settings->N);
//
//   return dataSet;
// }
