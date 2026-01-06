#pragma once
#include "core/bodies/particles.h"
#include "utils/namespaces/error_namespace.h"
#include <array>
#include <cstddef>
#include <cstdint>
#include <mutex>
#include <vector>

struct MortonKey {
  uint key_number;
};

class ParticleBlock {
public:
  ParticleBlock(uint morton_key, const std::vector<Particle> &particles);
  ParticleBlock(const ParticleBlock &) = delete;
  ParticleBlock() = default;

  ParticleBlock &operator=(const ParticleBlock &) = delete;
  ParticleBlock(ParticleBlock &&other) noexcept;
  ParticleBlock &operator=(ParticleBlock &&other) noexcept;
  ~ParticleBlock() = default;

  void swap(ParticleBlock &other) noexcept;
  void initialize() { debug::debug_print("we must impl part initialize"); }

  static constexpr int N{4 * 6};
  struct DataBlock {

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
    alignas(16) std::array<uint64_t, N> visual_id;

    unsigned short size = 0;

    DataBlock() noexcept = default;
    DataBlock(DataBlock &&other) noexcept = default;
    DataBlock &operator=(DataBlock &&other) noexcept = default;
#define DEFINE_GETTER(field)                                                   \
  const std::array<double, N> &get_##field() const { return field; }           \
  std::array<double, N> &get_##field() { return field; }
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
  };

  struct MetaBlock {
    MortonKey key;
    uint arena_block = 0;
    float hotness = 0.7f;
    uint next_logical_block;

    MetaBlock() noexcept = default;
    MetaBlock(MortonKey key) noexcept : key(key) {};
    MetaBlock(MetaBlock &&other) noexcept = default;
    MetaBlock &operator=(MetaBlock &&other) noexcept = default;
  };

  ParticleBlock *near_field[8];
  size_t addParticle(const Particle &p);
  Particle deleteParticle(size_t index);
  void printParticles();
  Particle getParticle(size_t index) const;
  MyMath::Vector3 getPosition(size_t index) const;

#define DEFINE_GETTER(field)                                                   \
  const std::array<double, N> &get_##field() const {                           \
    return data_block.field;                                                   \
  }                                                                            \
  std::array<double, N> &get_##field() { return data_block.field; }
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

  unsigned short size() const { return data_block.size; }
  bool is_full() const { return data_block.size >= N; }
  bool is_empty() const { return data_block.size == 0; }
  DataBlock data_block;
  MetaBlock meta_block;

private:
  mutable std::mutex m_mutex; // Мьютекс теперь в ParticleBlock
public:
  std::mutex &get_mutex() { return m_mutex; }
};
