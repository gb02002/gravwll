#pragma once

#include "core/bodies/particles.h"
#include "ctx/simulation_config.h"
#include "utils/namespaces/MyMath.h"
#include <memory>
#include <random>
#include <string>
#include <vector>

namespace {
struct PlummerGenerator {};
struct RandomGenerator {};
struct NoGenerator {};
} // namespace

class InitialConditionGenerator {
public:
  virtual ~InitialConditionGenerator() = default;

  virtual std::vector<Particle> get_data_set(int n_particles, int seed) = 0;
  virtual std::vector<Particle> get_data_set(int n_particles) {
    std::random_device rd;
    return get_data_set(n_particles, rd());
  };
};

class PlummerModelGenerator : public InitialConditionGenerator {
public:
  std::vector<Particle> get_data_set(int n_particles, int seed) override;

private:
  double total_mass_ = 1.0;
  double scale_radius_ = 1.0;
};

class UniformDistributionGenerator : public InitialConditionGenerator {
public:
  UniformDistributionGenerator(const MyMath::BoundingBox &initial_bb);
  std::vector<Particle> get_data_set(int n_particles, int seed) override;

private:
  MyMath::BoundingBox bounds_;
};

class DataSourceGenerator : public InitialConditionGenerator {
public:
  enum class SourceType {
    PLUMMER,
    UNIFORM,
    FILE_LOCAL,
    FILE_REMOTE,
    EMPTY,
  };
  DataSourceGenerator(SourceType type, const std::string &source_path = "");
  std::vector<Particle> get_data_set(int n_particles, int seed) override;
  void set_source_type(SourceType type) { source_type_ = type; };
  void set_source_path(const std::string &source_path) {
    source_path_ = source_path;
  };

private:
  std::string &source_path_;
  SourceType source_type_;
  std::unique_ptr<InitialConditionGenerator> create_delegate();
  std::vector<Particle> load_from_file(const std::string &filename);
  std::vector<Particle> fetch_for_remote(const std::string &url);
};

class GeneratorFactory {
public:
  static std::unique_ptr<InitialConditionGenerator>
  create_generator(const std::string &type, const std::string &source = "",
                   const MyMath::BoundingBox &bounds = {{0, 0, 0}, {1, 1, 1}});

  static std::unique_ptr<InitialConditionGenerator>
  create_from_config(const SimulationConfig &config);
};

// We don't generate, rather load from file or fetch
class DataLoader {
public:
  struct DatasetInfo {
    std::string name;
    std::string url;
    std::string format;
    size_t expected_particles;
  };

  static std::vector<Particle> load_from_file(const std::string &filename);
  static std::vector<Particle> download_dataset(const std::string &dataset_name,
                                                size_t max_particles = 0);
  static std::vector<DatasetInfo> get_available_datasets();

private:
  static std::vector<Particle> parse_hdf5(const std::string &filename);
  static std::vector<Particle> parse_csv(const std::string &filename);
};
