#pragma once
#include "core/bodies/particles.h"
#include <vector>

namespace generator_structs {

struct PlummerParams {
  double total_mass = 1.0;
  double scale_radius = 1.0;
  double G = 1.0;
  bool normilize_to_bb = false;
};
} // namespace generator_structs

// Простые функции вместо классов - идеально для scientific computing
namespace generators {

// Прямые функции для каждой модели
std::vector<Particle>
generate_plummer(size_t n, int seed,
                 const generator_structs::PlummerParams &params);
std::vector<Particle> generate_uniform(const MyMath::BoundingBox &box, size_t n,
                                       int seed = 42);
std::vector<Particle> generate_keplerian_disk(size_t n, int seed = 42);
std::vector<Particle> generate_empty();

} // namespace generators

namespace data_loader {

// Отдельный namespace для загрузки данных
std::vector<Particle> load_from_file(const std::string &filename);
std::vector<Particle> download_dataset(const std::string &dataset_name,
                                       size_t max_bodies = 0);

} // namespace data_loader
