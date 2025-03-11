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
  int FPS = 60;
  int IntegrationStep = 2000;

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
      {"--fps", "fps"},
      {"--integrationStep", "integrationstep"},
      {"--N", "n"},
      {"--n", "n"}};

  std::unordered_map<std::string, std::function<void(const std::string &)>>
      ARG_MAP = {
          {"headless", [this](const std::string &) { this->HEADLESS = true; }},
          {"verbose", [this](const std::string &) { this->VERBOSE = true; }},
          {"debug", [this](const std::string &) { this->DEBUG = true; }},

          {"treemaxdepth",
           [this](const std::string &val) {
             this->TreeMaxDepth = std::stoi(val);
           }},
          {"integrationstep",
           [this](const std::string &val) {
             this->IntegrationStep = std::stoi(val);
           }},
          {"fps",
           [this](const std::string &val) { this->FPS = std::stoi(val); }},
          {"datamode",
           [this](const std::string &val) {
             this->DataMode = StringToDataMode(val);
           }},
          {"n", [this](const std::string &val) { this->N = std::stoi(val); }}};
  const std::string ConfigFilePath = std::string(CONFIG_DIRECTORY);
};
