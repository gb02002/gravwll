#pragma once
#include "utils/namespaces/MyMath.h"

class Particle {
public:
  Particle(double x, double y, double z, double vx, double vy, double vz,
           double mass);
  Particle(double x, double y, double z, double vx, double vy, double vz,
           double fx, double fy, double fz, double ax, double ay, double az,
           double mass);

  ~Particle() = default;

  MyMath::Vector3 getPosition() const;
  double getMass() const;
  MyMath::Vector3 getVelocity() const;
  void setForce(const MyMath::Vector3 &f);

  double getX() const;
  double getY() const;
  double getZ() const;

  double getVx() const;
  double getVy() const;
  double getVz() const;

  double getForceX() const;
  double getForceY() const;
  double getForceZ() const;

  double getAx() const;
  double getAy() const;
  double getAz() const;

  void updateVelocity(double dvx, double dvy, double dvz);
  void move(double dx, double dy, double dz);
  void setForce(double fdx, double fdy, double fdz);
  void addForce(double fdx, double fdy, double fdz);

private:
  double x, y, z;
  double vx, vy, vz;
  double fx, fy, fz;
  double ax, ay, az;
  double mass;
};
