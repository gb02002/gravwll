#include "ds/tree/octree.h"
#include "ds/storage/storage.h"
#include <array>
#include <iostream>
#include <memory>
#include <mutex>
#include <vector>

AROctreeNode::AROctreeNode(MyMath::BoundingBox bounds, Multipole multipole,
                           const int depth, const int maxDepth,
                           Storage &storage)
    : bounds(bounds), multipole(multipole), depth(depth), maxDepth(maxDepth),
      storage(storage) {
  setCalculatedCenter(); // Вызываем метод уже после инициализации всех полей
  localBlock = storage.createMemBlock({});
};

AROctreeNode::AROctreeNode(const Cfx &cfx, Storage &storage)
    : bounds(cfx.InitialBB), multipole(Multipole{cfx.TotalMass}), depth(0),
      maxDepth(cfx.TreeMaxDepth), storage(storage) {};

AROctreeNode::~AROctreeNode() {
  for (auto &child : children) {
    delete child;
  }
}

void AROctreeNode::insert(const Particle &p, int index) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (depth == maxDepth) {
    localBlock->addParticle(p);
    // pointIndexes.push_back(index);
    return;
  }
  if (*children == nullptr) {
    if (localBlock->size < 1) {
      localBlock->addParticle(p);
    } else {
      this->split(p, index);
    }
    return;
  }
  auto bound_number = this->boundsCheck(p.getPosition());
  this->children[bound_number]->insert(p, index);
}

int AROctreeNode::boundsCheck(MyMath::Vector3 p) const {
  return ((p.x > center.x) << 2) | ((p.y > center.y) << 1) | (p.z > center.z);
}

void AROctreeNode::setCalculatedCenter() {
  center = {(bounds.max.x + bounds.min.x) * 0.5,
            (bounds.max.y + bounds.min.y) * 0.5,
            (bounds.max.z + bounds.min.z) * 0.5};
}

int AROctreeNode::getChildIndex(const MyMath::Vector3 &p) {
  return ((p.x >= center.x) << 2) | ((p.y >= center.y) << 1) |
         (p.z >= center.z);
}

std::array<MyMath::BoundingBox, 8> AROctreeNode::childBounds() {
  std::array<MyMath::BoundingBox, 8> childrenBoxes;
  for (int i = 0; i < 8; ++i) {
    MyMath::Vector3 newMin, newMax;
    // Определяем координаты для оси X:
    if (i & 4) { // если бит 2 = 1
      newMin.x = center.x;
      newMax.x = bounds.max.x;
    } else {
      newMin.x = bounds.min.x;
      newMax.x = center.x;
    }
    // Для оси Y:
    if (i & 2) { // если бит 1 = 1
      newMin.y = center.y;
      newMax.y = bounds.max.y;
    } else {
      newMin.y = bounds.min.y;
      newMax.y = center.y;
    }
    // Для оси Z:
    if (i & 1) { // если бит 0 = 1
      newMin.z = center.z;
      newMax.z = bounds.max.z;
    } else {
      newMin.z = bounds.min.z;
      newMax.z = center.z;
    }
    childrenBoxes[i] = MyMath::BoundingBox{newMin, newMax};
  }
  return childrenBoxes;
}

void AROctreeNode::split(const Particle &p, int index) {
  std::array<MyMath::BoundingBox, 8> childBoundingBoxes = childBounds();
  for (int i = 0; i < 8; ++i) {
    children[i] = new AROctreeNode(childBoundingBoxes[i], Multipole(),
                                   depth + 1, maxDepth, storage);
  }

  for (int n = 0; n < localBlock->size; ++n) {
    Particle tmp_p = localBlock->deleteParticle(n);
    int childIndex = boundsCheck(tmp_p.getPosition());
    children[childIndex]->insert(tmp_p, n);
  }
  // this->insert(p, index);
  int childIndex = boundsCheck(p.getPosition());
  children[childIndex]->insert(p, index);
  localBlock.reset();
  return;
}

void AROctree::insert(const Particle &p, int index) { root->insert(p, index); };

AROctree::~AROctree() { root.reset(); };

AROctree::AROctree(const Cfx &cfx, Storage &storage)
    : maxDepth(cfx.TreeMaxDepth), storage(storage) {
  std::cout << "The tree got initialized!\n";
  this->root = std::make_unique<AROctreeNode>(cfx, storage);
  std::cout << "Curr val: " << cfx.TreeMaxDepth << std::endl;
};

AROctreeNode *AROctree::get_root() { return root.get(); }

Multipole::Multipole(double mass) : totalMass(mass) {};

Multipole::Multipole() : totalMass(0) {};
