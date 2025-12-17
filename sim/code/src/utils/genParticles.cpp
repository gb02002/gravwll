#include "core/bodies/particles.h"
#include <random>
#include <vector>

std::vector<Particle> genRand(const int N) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<double> dis(
      0.03, 0.98); // диапазон значений от 0.03 до 0.98

  std::vector<Particle> particles;
  particles.reserve(N);

  for (int i = 0; i < N; ++i) {
    // Генерация случайных значений
    double x = dis(gen);
    double y = dis(gen);
    double z = dis(gen);

    // Применение квадратичной функции для усиления концентрации у краев
    x = (x < 0.5) ? x * x : 1.0 - (1.0 - x) * (1.0 - x);
    y = (y < 0.5) ? y * y : 1.0 - (1.0 - y) * (1.0 - y);
    z = (z < 0.5) ? z * z : 1.0 - (1.0 - z) * (1.0 - z);

    particles.emplace_back(x, y, z, dis(gen), dis(gen), dis(gen), dis(gen));
  }

  return particles;
}
