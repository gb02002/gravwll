#pragma once
#include "raylib.h"
#include <iostream>

namespace MyMath {
struct Vector3 {
  double x, y, z;

  Vector3() : x(0), y(0), z(0) {}
  Vector3(double x, double y, double z) : x(x), y(y), z(z) {}
  Vector3 operator-(const Vector3 &other) const {
    return {x - other.x, y - other.y, z - other.z};
  }
  Vector3 &operator=(const Vector3 &other) {
    if (this == &other)
      return *this;
    x = other.x;
    y = other.y;
    z = other.z;
    return *this;
  }
  operator ::Vector3() const {
    ::Vector3 v = {static_cast<float>(x), static_cast<float>(y),
                   static_cast<float>(z)};
    return v;
  }
  Vector3(const ::Vector3 &v) : x(v.x), y(v.y), z(v.z) {}
};
// Перегрузка оператора сложения для удобства
inline Vector3 operator+(const Vector3 &a, const Vector3 &b) {
  return {a.x + b.x, a.y + b.y, a.z + b.z};
}

// Перегрузка оператора умножения: вектор * скаляр
inline Vector3 operator*(const Vector3 &v, double s) {
  return {v.x * s, v.y * s, v.z * s};
}
// Также можно перегрузить оператор для случая: скаляр * вектор
inline Vector3 operator*(double s, const Vector3 &v) { return v * s; }

struct BoundingBox {
  MyMath::Vector3 min; // минимальные координаты по x, y, z
  MyMath::Vector3 max; // максимальные координаты по x, y, z
};
} // namespace MyMath
std::ostream &operator<<(std::ostream &out, MyMath::Vector3 const &vec);
std::ostream &operator<<(std::ostream &out, MyMath::BoundingBox const &bb);
