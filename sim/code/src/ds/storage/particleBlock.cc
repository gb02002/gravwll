#include "ds/storage/particleBlock.h"
#include "core/bodies/particles.h"
#include "iostream"
#include <mutex>
#include <utility>
#include <vector>

ParticleBlock::ParticleBlock(uint morton_key,
                             const std::vector<Particle> &particles)
    : meta_block(MortonKey{morton_key}), data_block() {
  for (const auto &p : particles) {
    addParticle(p);
  }
}

int ParticleBlock::addParticle(const Particle &p) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (is_full()) {
    std::cout << "We have exceeded the ParticleBlock size. Were not able to "
                 "add a Particle. Suck it!\n";
    return -1;
  }

  int index = data_block.size++;
  data_block.x[index] = p.getX();
  data_block.y[index] = p.getY();
  data_block.z[index] = p.getZ();
  data_block.vx[index] = p.getVx();
  data_block.vy[index] = p.getVy();
  data_block.vz[index] = p.getVz();
  data_block.fx[index] = p.getForceX();
  data_block.fy[index] = p.getForceY();
  data_block.fz[index] = p.getForceZ();
  data_block.ax[index] = p.getAx();
  data_block.ay[index] = p.getAy();
  data_block.az[index] = p.getAz();
  data_block.mass[index] = p.getMass();
  return index;
}

Particle ParticleBlock::deleteParticle(int index) {
  std::lock_guard<std::mutex> lock(m_mutex);

  if (index >= data_block.size || index < 0)
    return Particle{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};

  Particle p{
      data_block.x[index],    data_block.y[index],  data_block.z[index],
      data_block.vx[index],   data_block.vy[index], data_block.vz[index],
      data_block.fx[index],   data_block.fy[index], data_block.fz[index],
      data_block.ax[index],   data_block.ay[index], data_block.az[index],
      data_block.mass[index],
  };

  data_block.size--;

  if (index != data_block.size) {
    data_block.x[index] = data_block.x[data_block.size];
    data_block.y[index] = data_block.y[data_block.size];
    data_block.z[index] = data_block.z[data_block.size];
    data_block.vx[index] = data_block.vx[data_block.size];
    data_block.vy[index] = data_block.vy[data_block.size];
    data_block.vz[index] = data_block.vz[data_block.size];
    data_block.fx[index] = data_block.fx[data_block.size];
    data_block.fy[index] = data_block.fy[data_block.size];
    data_block.fz[index] = data_block.fz[data_block.size];
    data_block.ax[index] = data_block.ax[data_block.size];
    data_block.ay[index] = data_block.ay[data_block.size];
    data_block.az[index] = data_block.az[data_block.size];
    data_block.mass[index] = data_block.mass[data_block.size];
  }

  return p;
}

void ParticleBlock::printParticles() {
  std::lock_guard<std::mutex> lock(m_mutex);
  std::cout << "Particles in block (" << data_block.size << " particles):\n";
  for (int i = 0; i < data_block.size; ++i) {
    std::cout << "Particle " << i << ": x=" << data_block.x[i]
              << ", y=" << data_block.y[i] << ", z=" << data_block.z[i] << "\n";
  }
}

MyMath::Vector3 ParticleBlock::getPosition(int index) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (index < 0 || index >= data_block.size) {
    return {0.0, 0.0, 0.0};
  }
  return {data_block.x[index], data_block.y[index], data_block.z[index]};
}

Particle ParticleBlock::getParticle(int index) const {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (index < 0 || index >= data_block.size) {
    return Particle{0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0};
  }

  return Particle{
      data_block.x[index],    data_block.y[index],  data_block.z[index],
      data_block.vx[index],   data_block.vy[index], data_block.vz[index],
      data_block.fx[index],   data_block.fy[index], data_block.fz[index],
      data_block.ax[index],   data_block.ay[index], data_block.az[index],
      data_block.mass[index],
  };
}

ParticleBlock::ParticleBlock(ParticleBlock &&other) noexcept
    : data_block(std::move(other.data_block)),
      meta_block(std::move(other.meta_block)) {}

ParticleBlock &ParticleBlock::operator=(ParticleBlock &&other) noexcept {
  if (this != &other) {
    std::lock_guard<std::mutex> lock(m_mutex);
    data_block = std::move(other.data_block);
    meta_block = std::move(other.meta_block);
  }
  return *this;
}

void ParticleBlock::swap(ParticleBlock &other) noexcept {
  using std::swap;
  std::lock_guard<std::mutex> lock1(m_mutex);
  std::lock_guard<std::mutex> lock2(other.m_mutex);
  swap(data_block, other.data_block);
  swap(meta_block, other.meta_block);
}
