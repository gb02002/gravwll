#include "ds/storage/particleBlock.h"
#include "core/bodies/particles.h"
#include "iostream"
#include <mutex>
#include <vector>

ParticleBlock::ParticleBlock(const std::vector<Particle> &particles) {
  for (auto p : particles) {
    this->addParticle(p);
  }
};
ParticleBlock::~ParticleBlock() {};

int ParticleBlock::addParticle(const Particle &p) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (size >= N) {
    std::cout << "We have exceeded the ParticleBlock size. Were not able to "
                 "add a Particle. Suck it!\n";
    return 0;
  }

  int index = size++;
  x[index] = p.getX();
  y[index] = p.getY();
  z[index] = p.getZ();
  vx[index] = p.getVx();
  vy[index] = p.getVy();
  vz[index] = p.getVz();
  fx[index] = p.getForceX();
  fy[index] = p.getForceY();
  fz[index] = p.getForceZ();
  ax[index] = p.getAx();
  ay[index] = p.getAy();
  az[index] = p.getAz();
  mass[index] = p.getMass();
  return index;
};

// Not sure if we should return anything. tbd
Particle ParticleBlock::deleteParticle(int index) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (index >= size)
    return Particle{
        0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
    };
  size--;

  Particle p{
      x[size],  y[size],  z[size],  vx[size], vy[size], vz[size],   fx[size],
      fy[size], fz[size], ax[size], ay[size], az[size], mass[size],
  };
  x[index] = x[size];
  y[index] = y[size];
  z[index] = z[size];
  vx[index] = vx[size];
  vy[index] = vy[size];
  vz[index] = vz[size];
  fx[index] = fx[size];
  fy[index] = fy[size];
  fz[index] = fz[size];
  ax[index] = ax[size];
  ay[index] = ay[size];
  az[index] = az[size];
  mass[index] = mass[size];
  return p;
}

void ParticleBlock::printParticles() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::cout << "Particles in block (" << size << " particles):\n";
  for (int i = 0; i < size; ++i) {
    std::cout << "Particle " << i << ": x=" << x[i] << ", y=" << y[i]
              << ", z=" << z[i] << "\n";
  }
}

MyMath::Vector3 ParticleBlock::getPosition(int N) {
  std::lock_guard<std::mutex> lock(m_mutex);
  return {this->x[N], this->y[N], this->z[N]};
}
