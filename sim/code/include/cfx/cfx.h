#pragma once
#include "core/bodies/particles.h"
#include "settings/settings.h"
#include "utils/namespaces/MyMath.h"
#include <memory>
#include <vector>

enum SimState { RUN, STOP, EXIT };

class Cfx {
private:
  std::unique_ptr<Settings> settings;

public:
  Cfx(int argc, char **argv);
  Cfx();
  ~Cfx() = default;

  MyMath::BoundingBox InitialBB;
  const unsigned short TreeMaxDepth;
  double TotalMass;
  SimState state;

  std::vector<Particle> CreateDataSet();

private:
  void Init();
  void AdoptInitialValues();
};
