#include "ctx/dataSets.h"
#include <random>
#include <vector>

std::vector<Particle> GenerateRandomParticle(const int N) {
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<> dist(0.0, 1.0);

  std::vector<Particle> particles;
  for (auto i = 0; i < N; ++i)
    particles.push_back(
        Particle{dist(gen), dist(gen), dist(gen), 0.0, 0.0, 0.0, dist(gen)});

  return particles;
};
