#pragma once
#include "core/bodies/particles.h"
#include "settings/settings.h"
#include "utils/namespaces/MyMath.h"
#include <chrono>
#include <memory>
#include <vector>

enum SimState { RUN, STOP, EXIT };

class Cfx {
public:
  std::unique_ptr<Settings> settings;
  Cfx(int argc, char **argv);
  Cfx();
  ~Cfx() = default;

  MyMath::BoundingBox InitialBB;
  const unsigned short TreeMaxDepth;
  double TotalMass;
  SimState state;
  std::chrono::microseconds IntegrationStepInMicroseconds;

  std::vector<Particle> CreateDataSet();

private:
  void Init();
  void AdoptInitialValues();
};
