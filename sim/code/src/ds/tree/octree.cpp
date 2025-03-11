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
      maxDepth(cfx.TreeMaxDepth), storage(storage) {
  setCalculatedCenter(); // Вызываем метод уже после инициализации всех полей
  localBlock = storage.createMemBlock({});
};

AROctreeNode::~AROctreeNode() {
  for (auto &child : children) {
    delete child;
  }
}

void AROctreeNode::insert(const Particle &p) {
  std::lock_guard<std::mutex> lock(m_mutex);
  if (depth == maxDepth) {
    localBlock->addParticle(p);
    return;
  }
  if (*children == nullptr) {
    if (localBlock->size < 16) {
      localBlock->addParticle(p);
    } else {
      std::cout << "Мы сплипуемся";
      this->split(p);
    }
    return;
  }
  auto bound_number = this->boundsCheck(p.getPosition());
  this->children[bound_number]->insert(p);
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

void AROctreeNode::split(const Particle &p) {
  std::array<MyMath::BoundingBox, 8> childBoundingBoxes = childBounds();
  for (int i = 0; i < 8; ++i) {
    children[i] = new AROctreeNode(childBoundingBoxes[i], Multipole(),
                                   depth + 1, maxDepth, storage);
  }
  const int initialLbSize = localBlock->size;
  for (int n = 0; n < initialLbSize; ++n) {
    Particle tmp_p = localBlock->deleteParticle(0);
    int childIndex = boundsCheck(tmp_p.getPosition());
    children[childIndex]->insert(tmp_p);
  }

  int childIndex = boundsCheck(p.getPosition());
  children[childIndex]->insert(p);
  localBlock.reset();
  return;
}

void AROctree::insert(const Particle &p) { root->insert(p); };

AROctree::~AROctree() { root.reset(); };

AROctree::AROctree(const Cfx &cfx, Storage &storage)
    : maxDepth(cfx.TreeMaxDepth), storage(storage) {
  std::cout << "The tree got initialized!\n";
  this->root = std::make_unique<AROctreeNode>(cfx, storage);
  std::cout << "Curr val: " << cfx.settings->N << std::endl;
};

void AROctree::insert_batch(const std::vector<Particle> &dataSet) {
  for (auto p : dataSet)
    this->insert(p);
}

AROctreeNode *AROctree::get_root() { return root.get(); }

Multipole::Multipole(double mass) : totalMass(mass) {};

Multipole::Multipole() : totalMass(0) {};

void AROctreeNode::printOctreeMasses() {
  int nodeIndex = -1;
  const std::string indent = "";
  // Если узел задан с индексом, выводим его номер
  if (nodeIndex >= 0) {
    std::cout << indent << "child" << nodeIndex << " masses: ";
  } else {
    std::cout << indent << "root masses: ";
  }

  // Если узел содержит частички, выводим массы
  if (this->localBlock) {
    for (int i = 0; i < this->localBlock->size; ++i) {
      double m = this->localBlock->getParticle(i).getMass();
      std::cout << m;
      if (i < this->localBlock->size - 1)
        std::cout << ", ";
    }
  }
  std::cout << std::endl;

  // Если узел имеет дочерние узлы, обходим их
  for (int i = 0; i < 8; ++i) {
    if (this->children[i] != nullptr) {
      // Для наглядности добавляем отступ
      this->children[i]->printOctreeMasses();
    }
  }
}
