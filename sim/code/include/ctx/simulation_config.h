#pragma once

#include "config.h"
#include <functional>
#include <limits>
#include <stdexcept>
#include <string>
#include <sys/types.h>
#include <unordered_map>
#include <unordered_set>

namespace config {

class CommandLineParser {
public:
  struct ParsedResult {
    std::unordered_map<std::string, std::string> kw_pairs;
    std::unordered_set<std::string> flags;
  };

  CommandLineParser() = default;
  ParsedResult parse(int argc, char **argv);

private:
  std::unordered_map<std::string, std::string> arg_aliases_ = {
      {"--headless", "headless"},
      {"-h", "headless"},
      {"--verbose", "verbose"},
      {"-v", "verbose"},
      {"--debug", "debug"},
      {"--data-mode", "datamode"},
      {"-d", "datamode"},
      {"--fps", "fps"},
      {"--integrationStep", "integrationstep"},
      {"-is", "integrationstep"},
      {"--N_bodies", "n"},
      {"-n", "n"},
      {"-s", "seed"},
      {"-seed", "seed"}};
};

class ConfigFileReader {
public:
  struct ConfigData {
    std::unordered_map<std::string, std::string> values;
  };

  ConfigFileReader() = default;
  ConfigFileReader(const std::string &config_directory);
  ConfigData read_config(const std::string &filename);

private:
  std::string config_directory_ = CONFIG_DIRECTORY;
};
}; // namespace config

struct SimulationConfig {
  SimulationConfig();
  bool validate() const;
  bool kHeadless = false;
  uint kNBodies = 0;
  bool kVerbose = false;
  bool kDebug = false;
  unsigned short kTreeMaxDepth = 0;
  ushort kFpsDesired = 0;
  uint integration_step = 0;
  int random_seed = 0;
  std::string filename;
  std::string data_set_name;
  std::string fetch_url;

  enum PUPULATION_MODE {
    PLUMMER,
    KeplerianDisk,
    EMPTY,
    UNIFORM,
    FILE,
    FETCH,
    ERROR
  };

  PUPULATION_MODE data_population_mode = ERROR;

  PUPULATION_MODE from_string(const std::string &value);
  bool process_bools(const std::string &value);
};

// Must return raw values to be processed later in Context
class SimulationConfigBuilder {
public:
  SimulationConfigBuilder() {
    arg_map_ = {
        {"headless",
         [this](const std::string &val) {
           config_.kHeadless = config_.process_bools(val);
         }},
        {"verbose",
         [this](const std::string &val) {
           config_.kVerbose = config_.process_bools(val);
         }},
        {"debug",
         [this](const std::string &val) {
           config_.kDebug = config_.process_bools(val);
         }},
        {"treemaxdepth",
         [this](const std::string &val) {
           int value = std::stoi(val);
           if (value < 3)
             throw std::out_of_range("kTreeMaxDepth must be > 3");
           if (value > 30)
             throw std::out_of_range("kTreeMaxDepth is over 30. Rethink");
           config_.kTreeMaxDepth = static_cast<unsigned short>(value);
         }},
        {"integrationstep",
         [this](const std::string &val) {
           int value = std::stoi(val);
           if (value < 0)
             throw std::out_of_range("integrationstep must be > 0");
           if (value > std::numeric_limits<int>::max())
             throw std::out_of_range(
                 "integrationstep is too big for int to handle");
           config_.integration_step = static_cast<unsigned short>(value);
         }},
        {"fps",
         [this](const std::string &val) {
           int value = std::stoi(val);
           if (value < 0)
             throw std::out_of_range("fps must be > 0");
           if (value > std::numeric_limits<int>::max())
             throw std::out_of_range("fps is too big for int to handle");
           config_.kFpsDesired = static_cast<unsigned short>(value);
         }},
        {"datamode",
         [this](const std::string &val) {
           config_.data_population_mode = config_.from_string(val);
         }},
        {"n",
         [this](const std::string &val) {
           int value = std::stoi(val);
           if (value < 0)
             throw std::out_of_range("integrationstep must be > 0");
           if (value > std::numeric_limits<int>::max())
             throw std::out_of_range(
                 "integrationstep is too big for int to handle");
           config_.kNBodies = static_cast<uint>(value);
         }},
        {"seed", // добавлен обработчик для seed
         [this](const std::string &val) {
           config_.random_seed = std::stoi(val);
         }},
    };
  };

private:
  void
  apply_mappings(const std::unordered_map<std::string, std::string> &source);

public:
  SimulationConfigBuilder &with_defaults();
  SimulationConfigBuilder &
  with_config_file(const std::string &filename = "config.conf");
  SimulationConfigBuilder &with_command_line(int argc, char **argv);
  SimulationConfig build();

private:
  std::unordered_map<std::string, std::function<void(const std::string &)>>
      arg_map_;
  config::CommandLineParser cli_parser_;
  config::ConfigFileReader file_reader_;
  SimulationConfig config_;
};
