#include "engine/pairwise.h"
#include "ds/storage/particleBlock.h"
#include <chrono>
#include <cmath>
#include <cstddef>
#include <mutex>

#define SOFTENER 1e-20
#define G 6.67430 * 10e-11

void calcBlocskAx(ParticleBlock &block) {
  std::lock_guard<std::mutex> lock(block.get_mutex());
  for (size_t i = 0; i < block.data_block.size; ++i) {
    for (size_t j = i + 1; j < block.data_block.size; ++j) {
      if (i == j)
        continue;

      double dx = block.get_x()[j] - block.get_x()[i];
      double dy = block.get_y()[j] - block.get_y()[i];
      double dz = block.get_z()[j] - block.get_z()[i];
      double r2 = dx * dx + dy * dy + dz * dz + SOFTENER;
      double r = std::sqrt(r2);
      double inv_r = 1.0 / r;

      double forceMagnitude =
          G * block.get_mass()[i] * block.get_mass()[j] / r2;
      double fx = forceMagnitude * dx * inv_r;
      double fy = forceMagnitude * dy * inv_r;
      double fz = forceMagnitude * dz * inv_r;

      // Обновление ускорений с учётом массы и третьего закона Ньютона
      block.get_ax()[i] += fx / block.get_mass()[i];
      block.get_ay()[i] += fy / block.get_mass()[i];
      block.get_az()[i] += fz / block.get_mass()[i];

      block.get_ax()[j] -= fx / block.get_mass()[j];
      block.get_ay()[j] -= fy / block.get_mass()[j];
      block.get_az()[j] -= fz / block.get_mass()[j];
    }
  }
};

void updateCoords(ParticleBlock &block, std::chrono::microseconds dt) {
  std::lock_guard<std::mutex> lock(block.get_mutex());
  double dt_sec = (double)dt.count() * 1e-6;

  for (size_t i = 0; i < block.data_block.size; ++i) {
    block.get_vx()[i] += block.get_ax()[i] * dt_sec;
    block.get_vy()[i] += block.get_ay()[i] * dt_sec;
    block.get_vz()[i] += block.get_az()[i] * dt_sec;

    block.get_x()[i] += block.get_vx()[i] * dt_sec;
    block.get_y()[i] += block.get_vy()[i] * dt_sec;
    block.get_z()[i] += block.get_vz()[i] * dt_sec;
  }

  block.get_ax().fill(0);
  block.get_ay().fill(0);
  block.get_az().fill(0);
};
