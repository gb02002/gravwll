#include "ctx/simulation_config.h"
#include <algorithm>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <unordered_map>

void SimulationConfigBuilder::apply_mappings(
    const std::unordered_map<std::string, std::string> &source) {
  for (const auto &[key, value] : source) {
    auto it = arg_map_.find(key);
    if (it != arg_map_.end()) {
      it->second(value);
    }
  }
}

SimulationConfigBuilder &SimulationConfigBuilder::with_defaults() {
  // Установка значений по умолчанию
  config_.kHeadless = false;
  config_.kVerbose = false;
  config_.kDebug = false;
  config_.kTreeMaxDepth = 10;
  config_.integration_step = 1000;
  config_.kFpsDesired = 60;
  config_.data_population_mode = SimulationConfig::PUPULATION_MODE::PLUMMER;
  config_.kNBodies = 100;
  config_.random_seed = 42;
  return *this;
}

SimulationConfigBuilder &
SimulationConfigBuilder::with_config_file(const std::string &filename) {
  auto config_data = file_reader_.read_config(filename);
  apply_mappings(config_data.values);
  return *this;
}

SimulationConfigBuilder &
SimulationConfigBuilder::with_command_line(int argc, char **argv) {
  auto parsed = cli_parser_.parse(argc, argv);
  apply_mappings(parsed.kw_pairs);

  // Обработка флагов
  for (const auto &flag : parsed.flags) {
    auto it = arg_map_.find(flag);
    if (it != arg_map_.end()) {
      it->second(""); // Для флагов передаем пустую строку
    }
  }
  return *this;
}
SimulationConfig SimulationConfigBuilder::build() {
  if (!config_.validate()) {
    throw std::invalid_argument("Invalid configuration");
  }
  return config_;
}

config::ConfigFileReader::ConfigFileReader(
    const std::string &config_directory) {};

SimulationConfig::PUPULATION_MODE
SimulationConfig::from_string(const std::string &value) {
  std::string tmp_ = value;
  std::transform(value.begin(), value.end(), tmp_.begin(), ::tolower);

  if (tmp_ == "plummer")
    return PUPULATION_MODE::PLUMMER;
  if (tmp_ == "kepleriandisk")
    return PUPULATION_MODE::KeplerianDisk;
  if (tmp_ == "empty")
    return PUPULATION_MODE::EMPTY;
  if (tmp_ == "uniform")
    return PUPULATION_MODE::UNIFORM;
  if (tmp_ == "file")
    return PUPULATION_MODE::FILE;
  if (tmp_ == "fetch")
    return PUPULATION_MODE::FETCH;

  return PUPULATION_MODE::ERROR;
}

config::ConfigFileReader::ConfigData
config::ConfigFileReader::read_config(const std::string &filename) {
  std::string filepath =
      config_directory_.empty() ? filename : config_directory_ + "/" + filename;
  std::ifstream is_file(filepath);

  if (!is_file.is_open()) {
    throw std::runtime_error("Can't open config file: " + filepath);
  }

  ConfigData result;
  std::string line;

  while (std::getline(is_file, line)) {
    // Пропускаем комментарии и пустые строки
    size_t comment_pos = line.find('#');
    if (comment_pos != std::string::npos) {
      line = line.substr(0, comment_pos);
    }

    std::istringstream is_line(line);
    std::string key;
    if (std::getline(is_line, key, '=')) {
      std::string value;
      if (std::getline(is_line, value)) {
        // Убираем пробелы
        key.erase(0, key.find_first_not_of(" \t"));
        key.erase(key.find_last_not_of(" \t") + 1);
        value.erase(0, value.find_first_not_of(" \t"));
        value.erase(value.find_last_not_of(" \t") + 1);

        if (!key.empty()) {
          result.values[key] = value;
        }
      }
    }
  }

  return result;
}

config::CommandLineParser::ParsedResult
config::CommandLineParser::parse(int argc, char **argv) {
  ParsedResult result;

  for (int i = 1; i < argc; ++i) {
    std::string arg(argv[i]);
    std::string canonical_arg;

    // Ищем каноническое имя аргумента
    auto alias_it = arg_aliases_.find(arg);
    if (alias_it != arg_aliases_.end()) {
      canonical_arg = alias_it->second;
    } else {
      // Если аргумент не найден в алиасах, используем как есть (без префиксов)
      if (arg.size() > 2 && arg.substr(0, 2) == "--") {
        canonical_arg = arg.substr(2);
      } else if (arg.size() > 1 && arg[0] == '-') {
        canonical_arg = arg.substr(1);
      } else {
        std::cerr << "Unknown argument: " << arg << std::endl;
        continue;
      }
    }

    // Проверяем, является ли следующий аргумент значением
    if (i + 1 < argc) {
      std::string next_arg = argv[i + 1];
      if (next_arg[0] != '-') {
        result.kw_pairs[canonical_arg] = next_arg;
        ++i; // Пропускаем значение
        continue;
      }
    }

    // Если следующего аргумента нет или он начинается с '-', считаем флагом
    result.flags.insert(canonical_arg);
  }

  return result;
}

bool SimulationConfig::validate() const {
  // Добавьте необходимые проверки
  return integration_step > 0; // Пример проверки
}

SimulationConfig::SimulationConfig() = default;
