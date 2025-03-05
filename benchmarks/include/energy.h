#pragma once
#include "core/bodies/particles.h"
#include <vector>

double computeKineticEnergy(const std::vector<Particle> &particles);

double computePotentialEnergy(const std::vector<Particle> &particles);

double computeEnergyError(double E0, double EF);
