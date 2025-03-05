#include "core/bodies/particles.h"

Particle::Particle(double x, double y, double z, double vx, double vy,
                   double vz, double mass)
    : x(x), y(y), z(z), vx(vx), vy(vy), vz(vz), fx(0), fy(0), fz(0), ax(0),
      ay(0), az(0), mass((mass > 0) ? mass : -mass) {}

Particle::Particle(double x, double y, double z, double vx, double vy,
                   double vz, double fx, double fy, double fz, double ax,
                   double ay, double az, double mass)
    : x(x), y(y), z(z), vx(vx), vy(vy), vz(vz), fx(fx), fy(fy), fz(fz), ax(ax),
      ay(ay), az(az), mass((mass > 0) ? mass : -mass) {}

double Particle::getMass() const { return mass; }

double Particle::getX() const { return x; }
double Particle::getY() const { return y; }
double Particle::getZ() const { return z; }

double Particle::getForceX() const { return fx; }
double Particle::getForceY() const { return fy; }
double Particle::getForceZ() const { return fz; }

double Particle::getVx() const { return vx; }
double Particle::getVy() const { return vy; }
double Particle::getVz() const { return vz; }

double Particle::getAx() const { return ax; }
double Particle::getAy() const { return ay; }
double Particle::getAz() const { return az; }

void Particle::updateVelocity(double dvx, double dvy, double dvz) {
  vx += dvx;
  vy += dvy;
  vz += dvz;
}

void Particle::move(double dx, double dy, double dz) {
  x += dx;
  y += dy;
  z += dz;
}

void Particle::setForce(const MyMath::Vector3 &f) {
  fx = f.x;
  fy = f.y;
  fz = f.z;
}

void Particle::setForce(double fdx, double fdy, double fdz) {
  fx = fdx;
  fy = fdy;
  fz = fdz;
}

void Particle::addForce(double fdx, double fdy, double fdz) {
  fx += fdx;
  fy += fdy;
  fz += fdz;
}

MyMath::Vector3 Particle::getPosition() const {
  return MyMath::Vector3{x, y, z};
}
MyMath::Vector3 Particle::getVelocity() const {
  return MyMath::Vector3{vx, vy, vz};
}
