#include "cfx/cfx.h"
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
}

std::vector<Particle> Cfx::CreateDataSet() {
  auto dataSet = std::vector<Particle>();
  return dataSet;
}
