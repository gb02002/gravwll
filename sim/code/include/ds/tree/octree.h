#pragma once
#include "cfx/cfx.h"
#include "ds/storage/storage.h"
#include <array>
#include <memory>
#include <mutex>

struct Multipole {
  double totalMass; // суммарная масса точек в узле Vector3 centerOfMass; //
                    // центр масс для вычислений
  // Дополнительно можно добавить квадрупольные или более высокие моменты
  Multipole(double mass);
  Multipole();
};

struct AROctreeNode {
  friend class AROctree;

  MyMath::BoundingBox bounds;
  AROctreeNode *children[8] = {nullptr};
  Multipole multipole;
  int depth;
  MyMath::Vector3 center;

  AROctreeNode(MyMath::BoundingBox bounds, Multipole multipole, const int depth,
               const int maxDepth, Storage &storage);
  AROctreeNode(const Cfx &cfx, Storage &storage);
  ~AROctreeNode();
  std::mutex &getMutex() const { return m_mutex; };

  std::unique_ptr<ParticleBlock> localBlock;

private:
  mutable std::mutex m_mutex;
  int maxDepth;
  Storage &storage;

  void setCalculatedCenter();
  void insert(const Particle &P, int index);
  void split(const Particle &P, int index);
  int boundsCheck(MyMath::Vector3 p) const;
  std::array<MyMath::BoundingBox, 8> childBounds();
  int getChildIndex(const MyMath::Vector3 &p);
};

class AROctree {
public:
  AROctree(const Cfx &cfx, Storage &storage);
  ~AROctree();

  void insert(const Particle &p, int index);
  void print();
  AROctreeNode *get_root();

private:
  std::unique_ptr<AROctreeNode> root;
  int maxDepth;
  Storage &storage;
  int minPointsPerNode;
};
