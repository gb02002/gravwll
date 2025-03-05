#pragma once
#include "config.h"
#include <functional>
#include <string>

// Исправляем название enum (если это опечатка)
enum PopuationDataMode { CONFIG, NO_DATA, GEN };

// Объявляем функцию для конвертации строки в enum
PopuationDataMode StringToDataMode(
    const std::string &str); // extern const std::string @CONFIG_DIRECTORY;
class Settings {
public:
  Settings(int argc, char **argv);
  Settings() { SetDefault(); };

  bool HEADLESS;
  int N;
  bool VERBOSE;
  bool DEBUG;
  unsigned short TreeMaxDepth;
  PopuationDataMode DataMode;

private:
  void ReadInput(int argc, char **argv);
  void SetDefault();
  void ReadConfigFile();
  void SetUpValue(std::string &key, std::string &value);

  std::unordered_map<std::string, std::string> ARG_ALIAS = {
      {"--headless", "headless"},
      {"-h", "headless"},
      {"--verbose", "verbose"},
      {"-v", "verbose"},
      {"--debug", "debug"},
      {"-d", "debug"},
      {"--data-mode", "datamode"},
      {"-dm", "datamode"},
      {"--N", "N"}};

  std::unordered_map<std::string, std::function<void(const std::string &)>>
      ARG_MAP = {
          {"headless", [this](const std::string &) { this->HEADLESS = true; }},
          {"verbose", [this](const std::string &) { this->VERBOSE = true; }},
          {"debug", [this](const std::string &) { this->DEBUG = true; }},

          {"treemaxdepth",
           [this](const std::string &val) {
             this->TreeMaxDepth = std::stoi(val);
           }},
          {"datamode",
           [this](const std::string &val) {
             this->DataMode = StringToDataMode(val);
           }},
          {"N", [this](const std::string &val) { this->N = std::stoi(val); }}};
  const std::string ConfigFilePath = std::string(CONFIG_DIRECTORY);
};
