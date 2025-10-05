#pragma once
#include "core/bodies/particles.h"
#include <array>
#include <mutex>
#include <vector>

struct MortonKey {
  uint key_number;
};

class ParticleBlock {
public:
  ParticleBlock(uint morton_key, const std::vector<Particle> &particles);
  ParticleBlock(const ParticleBlock &) = delete;

  ParticleBlock &operator=(const ParticleBlock &) = delete;
  ParticleBlock(ParticleBlock &&other) noexcept;
  ParticleBlock &operator=(ParticleBlock &&other) noexcept;
  ~ParticleBlock();

  void swap(ParticleBlock &other) noexcept;
  void initialize() { std::cout << "we must impl part initialize\n"; }

  struct DataBlock {
    // ParticleBlock::DataBlock(const std::vector<Particle> &particles);
    static constexpr int N{4 * 6};

    int addParticle(const Particle &p);
    Particle deleteParticle(int index);

    std::mutex &getMutex() const { return m_mutex; };

    void printParticles();

    Particle getParticle(int index);
    // Универсальный шаблонный геттер, возвращающий ссылку на поле по указателю
    // на член
    template <std::array<double, N> DataBlock::*Member>
    std::array<double, N> &getField() {
      return this->*Member;
    }

    template <std::array<double, N> DataBlock::*Member>
    const std::array<double, N> &getField() const {
      return this->*Member;
    }

    // Макрос для определения публичных геттеров для приватных полей
    // Например, get_x() для поля x.
#define DEFINE_GETTER(field)                                                   \
  const std::array<double, N> &get_##field() const {                           \
    return getField<&DataBlock::field>();                                      \
  }                                                                            \
  std::array<double, N> &get_##field() { return getField<&DataBlock::field>(); }
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

  public:
    DataBlock(DataBlock &&other) noexcept
        : x(std::move(other.x)), y(std::move(other.y)), z(std::move(other.z)),
          vx(std::move(other.vx)), vy(std::move(other.vy)),
          vz(std::move(other.vz)), fx(std::move(other.fx)),
          fy(std::move(other.fy)), fz(std::move(other.fz)),
          ax(std::move(other.ax)), ay(std::move(other.ay)),
          az(std::move(other.az)), mass(std::move(other.mass)),
          size(other.size) {
      other.size = 0;
    }
    DataBlock() noexcept : size(0) {}

    // Оператор перемещающего присваивания
    DataBlock &operator=(DataBlock &&other) noexcept {
      if (this != &other) {
        x = std::move(other.x);
        y = std::move(other.y);
        z = std::move(other.z);
        vx = std::move(other.vx);
        vy = std::move(other.vy);
        vz = std::move(other.vz);
        fx = std::move(other.fx);
        fy = std::move(other.fy);
        fz = std::move(other.fz);
        ax = std::move(other.ax);
        ay = std::move(other.ay);
        az = std::move(other.az);
        mass = std::move(other.mass);
        size = other.size;
        other.size = 0;
      }
      return *this;
    }
  };
  // key is filled during downward call from tree&leaf and finished with
  // upward call with next_logical_block and arena_block
  struct MetaBlock {
    MortonKey key;
    uint arena_block = -1;
    float hotness = 0.7;
    uint next_logical_block; // Despite each leaf having 26 neighboars in IL,
                             // this represents memory layout
    MetaBlock() noexcept = default;
    MetaBlock(MetaBlock &&other) noexcept = default;
    MetaBlock &operator=(MetaBlock &&other) noexcept = default;
  };
  DataBlock data_block;
  MetaBlock meta_block;
};
