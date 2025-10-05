#include "utils/generators.h"
#include "core/bodies/particles.h"
#include <random>

double generate_plummer_radius(double u) {
  // Радиус по обратному преобразованию от кумулятивного распределения
  // P(<r) = (r^3) / (1 + r^2)^(3/2) для безразмерного радиуса
  return std::pow(u, 2.0 / 3.0) / std::sqrt(1.0 - std::pow(u, 2.0 / 3.0));
}

std::tuple<double, double, double> generate_isotropic_direction(double u1,
                                                                double u2) {
  // Изотропное направление в сферических координатах
  double theta = 2.0 * M_PI * u1;         // Азимутальный угол [0, 2π]
  double phi = std::acos(2.0 * u2 - 1.0); // Полярный угол [0, π]

  double sin_phi = std::sin(phi);
  return {sin_phi * std::cos(theta), sin_phi * std::sin(theta), std::cos(phi)};
}

MyMath::Vector3
generate_plummer_velocity(double r, double u1, double u2,
                          const generator_structs::PlummerParams &params,
                          std::mt19937 &rng) {
  // Метод отвержения для генерации скорости в модели Пламмера
  double escape_velocity =
      std::sqrt(2.0 * params.G * params.total_mass /
                std::sqrt(r * r + params.scale_radius * params.scale_radius));

  double v;
  do {
    double u3 = u1; // переиспользуем существующие случайные числа
    double u4 = u2;

    v = u3 * escape_velocity;
    double f = u4 * 0.1; // эмпирическая верхняя граница для PDF

    // Функция распределения скоростей Пламмера
    double pdf =
        std::pow(1.0 - (v * v) / (escape_velocity * escape_velocity), 3.5);

    if (f <= pdf)
      break;

    // Если отвергли, генерируем новые числа
    u1 = std::generate_canonical<double, 10>(rng);
    u2 = std::generate_canonical<double, 10>(rng);

  } while (true);

  // Случайное направление для скорости
  auto [vx, vy, vz] =
      generate_isotropic_direction(std::generate_canonical<double, 10>(rng),
                                   std::generate_canonical<double, 10>(rng));

  return {v * vx, v * vy, v * vz};
}

void center_system(std::vector<Particle> &particles) {
  // Центрирование системы по центру масс
  MyMath::Vector3 com_position = {0, 0, 0};
  MyMath::Vector3 com_velocity = {0, 0, 0};
  double total_mass = 0.0;

  for (const auto &p : particles) {
    com_position.x += p.getX() * p.getMass();
    com_position.y += p.getY() * p.getMass();
    com_position.z += p.getZ() * p.getMass();

    com_velocity.x += p.getX() * p.getMass();
    com_velocity.y += p.getY() * p.getMass();
    com_velocity.z += p.getZ() * p.getMass();

    total_mass += p.getMass();
  }

  if (total_mass > 0) {
    com_position.x /= total_mass;
    com_position.y /= total_mass;
    com_position.z /= total_mass;

    com_velocity.x /= total_mass;
    com_velocity.y /= total_mass;
    com_velocity.z /= total_mass;

    // Сдвигаем все частицы
    for (auto &p : particles) {
      p.move(-com_position.x, -com_position.y, -com_position.z);
      p.update_velocity(-com_velocity.x, -com_velocity.y, -com_velocity.z);
    }
  }
}

namespace generators {

std::vector<Particle> generate_keplerian_disk(size_t n, int seed) { return {}; }

std::vector<Particle> generate_empty() { return {}; }

std::vector<Particle>
generate_plummer(size_t n, int seed = 42,
                 const generator_structs::PlummerParams &params =
                     generator_structs::PlummerParams{}) {
  std::vector<Particle> particles;
  particles.reserve(n);

  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> uniform(0.0, 1.0);

  const double mass_per_particle = params.total_mass / n;

  for (size_t i = 0; i < n; ++i) {
    // 1. Генерация радиуса по распределению Пламмера
    double r = generate_plummer_radius(uniform(rng));

    // 2. Изотропное направление
    auto [x, y, z] = generate_isotropic_direction(uniform(rng), uniform(rng));

    // 3. Позиция с масштабированием
    MyMath::Vector3 position = {r * x * params.scale_radius,
                                r * y * params.scale_radius,
                                r * z * params.scale_radius};

    // 4. Генерация скорости (по распределению Пламмера)
    auto velocity =
        generate_plummer_velocity(r, uniform(rng), uniform(rng), params, rng);

    particles.push_back(Particle{position, velocity, mass_per_particle});
  }

  // 5. Центрирование системы (опционально, но полезно)
  center_system(particles);

  return particles;
}

std::vector<Particle> generate_uniform(const MyMath::BoundingBox &box, size_t n,
                                       int seed) {
  std::vector<Particle> particles;
  particles.reserve(n);

  std::mt19937 rng(seed);
  std::uniform_real_distribution<double> dist_x(box.min.x, box.max.x);
  std::uniform_real_distribution<double> dist_y(box.min.y, box.max.y);
  std::uniform_real_distribution<double> dist_z(box.min.z, box.max.z);

  for (size_t i = 0; i < n; ++i) {
    particles.push_back(
        Particle{dist_x(rng), dist_y(rng), dist_z(rng), 0, 0, 0, 1.0});
  }

  return particles;
}

// ... остальные реализации

} // namespace generators

namespace data_loader {

std::vector<Particle> load_from_file(const std::string &filename) {
  // Простая загрузка из файла
  std::vector<Particle> particles;
  // ... реализация
  return particles;
}

std::vector<Particle> download_dataset(const std::string &dataset_name,
                                       size_t max_bodies) {
  // Загрузка датасета
  std::vector<Particle> particles;
  // ... реализация
  return particles;
}

} // namespace data_loader
