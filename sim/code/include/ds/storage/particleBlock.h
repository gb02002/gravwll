#pragma once
#include "core/bodies/particles.h"
#include <array>
#include <mutex>
#include <vector>

class ParticleBlock {
public:
  static constexpr int N{4 * 6};

  ParticleBlock(const std::vector<Particle> &particles);
  ~ParticleBlock();

  int addParticle(const Particle &p);
  Particle deleteParticle(int index);

  std::mutex &getMutex() const { return m_mutex; };

  void printParticles();

  // Универсальный шаблонный геттер, возвращающий ссылку на поле по указателю на
  // член
  template <std::array<double, N> ParticleBlock::*Member>
  std::array<double, N> &getField() {
    return this->*Member;
  }

  template <std::array<double, N> ParticleBlock::*Member>
  const std::array<double, N> &getField() const {
    return this->*Member;
  }

  // Макрос для определения публичных геттеров для приватных полей
  // Например, get_x() для поля x.
#define DEFINE_GETTER(field)                                                   \
  const std::array<double, N> &get_##field() const {                           \
    return getField<&ParticleBlock::field>();                                  \
  }                                                                            \
  std::array<double, N> &get_##field() {                                       \
    return getField<&ParticleBlock::field>();                                  \
  }

  DEFINE_GETTER(x)
  DEFINE_GETTER(y)
  DEFINE_GETTER(z)
  DEFINE_GETTER(vx)
  DEFINE_GETTER(vy)
  DEFINE_GETTER(vz)
  DEFINE_GETTER(fx)
  DEFINE_GETTER(fy)
  DEFINE_GETTER(fz)
  DEFINE_GETTER(ax)
  DEFINE_GETTER(ay)
  DEFINE_GETTER(az)
  DEFINE_GETTER(mass)

#undef DEFINE_GETTER
  int size = 0;
  MyMath::Vector3 getPosition(int N);

private:
  mutable std::mutex m_mutex;

  alignas(16) std::array<double, N> x;
  alignas(16) std::array<double, N> y;
  alignas(16) std::array<double, N> z;

  alignas(16) std::array<double, N> vx;
  alignas(16) std::array<double, N> vy;
  alignas(16) std::array<double, N> vz;

  alignas(16) std::array<double, N> fx;
  alignas(16) std::array<double, N> fy;
  alignas(16) std::array<double, N> fz;

  alignas(16) std::array<double, N> ax;
  alignas(16) std::array<double, N> ay;
  alignas(16) std::array<double, N> az;

  alignas(16) std::array<double, N> mass;
};
