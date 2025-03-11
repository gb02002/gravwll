#include "cfx/cfx.h"
#include "cfx/dataSets.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <vector>

Cfx::Cfx()
    : settings(std::make_unique<Settings>()),
      TreeMaxDepth(settings->TreeMaxDepth) {
  Init();
}

Cfx::Cfx(int argc, char **argv)
    : settings(std::make_unique<Settings>(argc, argv)),
      TreeMaxDepth(settings->TreeMaxDepth) {
  Init();
}

void Cfx::Init() { AdoptInitialValues(); }

void Cfx::AdoptInitialValues() {
  std::cout << "In this function we need to retrive data from settings and "
               "prepare them for cfx\n";

  InitialBB = {{0, 0, 0}, {1, 1, 1}};
  IntegrationStepInMicroseconds =
      std::chrono::microseconds(settings->IntegrationStep);
}

// Plammer to be implemented later
std::vector<Particle> Cfx::CreateDataSet() {
  auto dataSet = GenerateRandomParticle(this->settings->N);

  return dataSet;
}
