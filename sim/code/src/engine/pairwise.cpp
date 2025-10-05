#include "engine/pairwise.h"
#include "ds/storage/particleBlock.h"
#include <chrono>
#include <cmath>
#include <mutex>

#define SOFTENER 1e-20
#define G 6.67430 * 10e-11

void calcBlocskAx(ParticleBlock &block) {
  std::lock_guard<std::mutex> lock(block.data_block.getMutex());
  for (int i = 0; i < block.data_block.size; ++i) {
    for (int j = i + 1; j < block.data_block.size; ++j) {
      if (i == j)
        continue;

      double dx = block.data_block.get_x()[j] - block.data_block.get_x()[i];
      double dy = block.data_block.get_y()[j] - block.data_block.get_y()[i];
      double dz = block.data_block.get_z()[j] - block.data_block.get_z()[i];
      double r2 = dx * dx + dy * dy + dz * dz + SOFTENER;
      double r = std::sqrt(r2);
      double inv_r = 1.0 / r;

      double forceMagnitude = G * block.data_block.get_mass()[i] *
                              block.data_block.get_mass()[j] / r2;
      double fx = forceMagnitude * dx * inv_r;
      double fy = forceMagnitude * dy * inv_r;
      double fz = forceMagnitude * dz * inv_r;

      // Обновление ускорений с учётом массы и третьего закона Ньютона
      block.data_block.get_ax()[i] += fx / block.data_block.get_mass()[i];
      block.data_block.get_ay()[i] += fy / block.data_block.get_mass()[i];
      block.data_block.get_az()[i] += fz / block.data_block.get_mass()[i];

      block.data_block.get_ax()[j] -= fx / block.data_block.get_mass()[j];
      block.data_block.get_ay()[j] -= fy / block.data_block.get_mass()[j];
      block.data_block.get_az()[j] -= fz / block.data_block.get_mass()[j];
    }
  }
};

void updateCoords(ParticleBlock &block, std::chrono::microseconds dt) {
  std::lock_guard<std::mutex> lock(block.data_block.getMutex());
  double dt_sec = dt.count() * 1e-6;

  for (int i = 0; i < block.data_block.size; ++i) {
    block.data_block.get_vx()[i] += block.data_block.get_ax()[i] * dt_sec;
    block.data_block.get_vy()[i] += block.data_block.get_ay()[i] * dt_sec;
    block.data_block.get_vz()[i] += block.data_block.get_az()[i] * dt_sec;

    block.data_block.get_x()[i] += block.data_block.get_vx()[i] * dt_sec;
    block.data_block.get_y()[i] += block.data_block.get_vy()[i] * dt_sec;
    block.data_block.get_z()[i] += block.data_block.get_vz()[i] * dt_sec;
  }

  block.data_block.get_ax().fill(0);
  block.data_block.get_ay().fill(0);
  block.data_block.get_az().fill(0);
};
