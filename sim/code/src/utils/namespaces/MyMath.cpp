#include <iostream>
#include <utils/namespaces/MyMath.h>

std::ostream &operator<<(std::ostream &out, MyMath::Vector3 const &vec) {
  out << "{";
  out << vec.x << ";";
  out << vec.y << ";";
  out << vec.z << "}";
  return out;
};

std::ostream &operator<<(std::ostream &out, MyMath::BoundingBox const &bb) {
  out << bb.min << ":" << bb.max;
  return out;
};
