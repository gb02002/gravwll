#include "energy.h"
#include "core/bodies/particles.h"
#include <cmath>
#include <vector>

const double G = 6.67430e-11;

double computeKineticEnergy(const std::vector<Particle> &particles) {
  double totalKE{0.0};
  for (const auto &p : particles) {
    totalKE +=
        0.5 * p.getMass() * (p.getSX() * p.getSX() + p.getSY() * p.getSY());
  }
  return totalKE;
}

double computePotentialEnergy(const std::vector<Particle> &particles) {
  double totalPE{0.0};
  for (size_t i = 0; i < particles.size(); ++i) {
    for (size_t j = i + 1; j < particles.size(); ++j) {

      double dx = particles[i].getX() - particles[j].getX();
      double dy = particles[i].getY() - particles[j].getY();
      double r = sqrt(dx * dx + dy * dy);

      if (r > 1e-6) {
        totalPE -= G * particles[i].getMass() * particles[j].getMass() / r;
      }
    }
  }
  return totalPE;
}

double computeEnergyError(double E0, double EF) {
  return fabs((EF - E0) / E0) * 100.0;
}
