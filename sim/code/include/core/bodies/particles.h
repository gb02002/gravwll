#pragma once
#include "utils/namespaces/MyMath.h"
#include <cstdint>

class Particle {
public:
  Particle(double x, double y, double z, double vx, double vy, double vz,
           double mass, uint64_t visual_id)
      : x(x), y(y), z(z), vx(vx), vy(vy), vz(vz), fx(0), fy(0), fz(0), ax(0),
        ay(0), az(0), mass((mass > 0) ? mass : -mass), visual_id(visual_id) {}

  Particle(double x, double y, double z, double vx, double vy, double vz,
           double mass)
      : x(x), y(y), z(z), vx(vx), vy(vy), vz(vz), fx(0), fy(0), fz(0), ax(0),
        ay(0), az(0), mass((mass > 0) ? mass : -mass) {}

  Particle(double x, double y, double z, double vx, double vy, double vz,
           double fx, double fy, double fz, double ax, double ay, double az,
           double mass)
      : x(x), y(y), z(z), vx(vx), vy(vy), vz(vz), fx(fx), fy(fy), fz(fz),
        ax(ax), ay(ay), az(az), mass((mass > 0) ? mass : -mass) {}

  Particle(MyMath::Vector3 position, MyMath::Vector3 velocity, double mass)
      : x(position.x), y(position.y), z(position.z), vx(velocity.x),
        vy(velocity.y), vz(velocity.z), fx(0), fy(0), fz(0), ax(0), ay(0),
        az(0), mass((mass > 0) ? mass : -mass) {};

  ~Particle() = default;

  inline MyMath::Vector3 getPosition() const;
  inline double getMass() const;
  inline MyMath::Vector3 getVelocity() const;
  inline void setForce(const MyMath::Vector3 &f);

  inline double getX() const;
  inline double getY() const;
  inline double getZ() const;

  inline double getVx() const;
  inline double getVy() const;
  inline double getVz() const;

  // inline double getX();
  // inline double getY();
  // inline double getZ();
  //
  // inline double getVx();
  // inline double getVy();
  // inline double getVz();

  inline double getForceX() const;
  inline double getForceY() const;
  inline double getForceZ() const;

  inline double getAx() const;
  inline double getAy() const;
  inline double getAz() const;

  inline void update_velocity(double dvx, double dvy, double dvz);
  inline void update_position(double dx, double dy, double dz);
  inline void move(double dx, double dy, double dz);
  inline void setForce(double fdx, double fdy, double fdz);
  inline void addForce(double fdx, double fdy, double fdz);

private:
  double x, y, z;
  double vx, vy, vz;
  double fx, fy, fz;
  double ax, ay, az;
  double mass;
  uint64_t visual_id;
};
inline std::ostream &operator<<(std::ostream &out, Particle const &p);

inline double Particle::getMass() const { return mass; }

inline double Particle::getX() const { return x; }
inline double Particle::getY() const { return y; }
inline double Particle::getZ() const { return z; }

// double Particle::getX() { return x; }
// double Particle::getY() { return y; }
// double Particle::getZ() { return z; }

inline double Particle::getForceX() const { return fx; }
inline double Particle::getForceY() const { return fy; }
inline double Particle::getForceZ() const { return fz; }

inline double Particle::getVx() const { return vx; }
inline double Particle::getVy() const { return vy; }
inline double Particle::getVz() const { return vz; }

inline double Particle::getAx() const { return ax; }
inline double Particle::getAy() const { return ay; }
inline double Particle::getAz() const { return az; }

inline void Particle::update_velocity(double dvx, double dvy, double dvz) {
  vx += dvx;
  vy += dvy;
  vz += dvz;
}

inline void Particle::move(double dx, double dy, double dz) {
  x += dx;
  y += dy;
  z += dz;
}

inline void Particle::setForce(const MyMath::Vector3 &f) {
  fx = f.x;
  fy = f.y;
  fz = f.z;
}

inline void Particle::setForce(double fdx, double fdy, double fdz) {
  fx = fdx;
  fy = fdy;
  fz = fdz;
}

inline void Particle::addForce(double fdx, double fdy, double fdz) {
  fx += fdx;
  fy += fdy;
  fz += fdz;
}

inline MyMath::Vector3 Particle::getPosition() const {
  return MyMath::Vector3{x, y, z};
}
inline MyMath::Vector3 Particle::getVelocity() const {
  return MyMath::Vector3{vx, vy, vz};
}

std::ostream &operator<<(std::ostream &out, Particle const &p) {
  out << "Particle with " << p.getMass() << " mass ";
  out << "has coords: {";
  out << p.getX() << ":";
  out << p.getY() << ":";
  out << p.getZ() << "}";
  return out;
};
