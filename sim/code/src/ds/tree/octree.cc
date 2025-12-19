#include <algorithm>
#define CURRENT_MODULE_DEBUG 0
#include "ds/storage/storage.h"
#include "ds/tree/octree.h"
#include "gfx/renderer/scene.h"
#include "utils/namespaces/MyMath.h"
#include "utils/namespaces/error_namespace.h"
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
  setCalculatedCenter();
  localBlock = storage.create_memory_block(1, {});
};

AROctreeNode::AROctreeNode(MyMath::BoundingBox &prime_bounds,
                           const unsigned short tree_max_depth,
                           Storage &storage)
    : bounds(prime_bounds), multipole(Multipole{}), depth(0),
      maxDepth(tree_max_depth), storage(storage) {
  setCalculatedCenter();
  localBlock = storage.create_memory_block(1, {});
};

AROctreeNode::~AROctreeNode() {
  for (auto &child : children) {
    delete child;
  }
}

void AROctreeNode::insert(const Particle &p) {
  std::lock_guard<std::mutex> lock(m_mutex);
  debug::debug_print("Мы в инверте");
  if (depth == maxDepth) {
    localBlock->addParticle(p);
    return;
  }
  if (*children == nullptr) {
    if (localBlock->data_block.size < 16) {
      localBlock->addParticle(p);
    } else {
      debug::debug_print("Мы сплипуемся");
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

// TODO must get an array for each layer. Not complex as the size is fixed
void AROctreeNode::split(const Particle &p) {
  std::array<MyMath::BoundingBox, 8> childBoundingBoxes = childBounds();
  for (int i = 0; i < 8; ++i) {
    children[i] = new AROctreeNode(childBoundingBoxes[i], Multipole(),
                                   depth + 1, maxDepth, storage);
  }
  const int size_of_initial_block = localBlock->data_block.size;
  for (int n = 0; n < size_of_initial_block; ++n) {
    Particle tmp_p = localBlock->deleteParticle(0);
    int childIndex = boundsCheck(tmp_p.getPosition());
    children[childIndex]->insert(tmp_p);
  }

  int childIndex = boundsCheck(p.getPosition());
  children[childIndex]->insert(p);
  storage.release_block(this->localBlock);
  localBlock = nullptr;
  return;
}

void AROctree::insert(const Particle &p) { root->insert(p); };

AROctree::~AROctree() { root.reset(); };

AROctree::AROctree(unsigned short max_tree_depth,
                   MyMath::BoundingBox prime_bounds, Storage &storage)
    : maxDepth(max_tree_depth), storage(storage) {
  std::cout << "The tree got initialized!\n";
  this->root =
      std::make_unique<AROctreeNode>(prime_bounds, max_tree_depth, storage);
};

void AROctree::insert_batch(const std::vector<Particle> &dataSet) {
  debug::debug_print("insert_batch 11111, len dataSet: {}", dataSet.size());
  for (auto p : dataSet) {
    this->insert(p);
  }
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
    for (int i = 0; i < this->localBlock->data_block.size; ++i) {
      double m = this->localBlock->getParticle(i).getMass();
      std::cout << m;
      if (i < this->localBlock->data_block.size - 1)
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

// WARN: We leak Scne objects here. Convertions must be handled on gfx side
std::vector<gfx::renderer::SceneParticle> &
AROctree::get_particles_for_render() {
  // std::vector<gfx::renderer::SceneParticle> vals{};
  static std::vector<gfx::renderer::SceneParticle> vals{};
  vals.resize(10000);
  gfx::renderer::SceneParticle e{};
  std::fill(vals.begin(), vals.end(), e);
  for (int i = 0; i < 10000; ++i) {
    vals[i] = {.position =
                   glm::vec3((rand() / (float)RAND_MAX - 0.5f) * 100.0f,
                             (rand() / (float)RAND_MAX - 0.5f) * 100.0f,
                             (rand() / (float)RAND_MAX - 0.5f) * 100.0f),
               .mass = 1.0f + (rand() / (float)RAND_MAX) * 10.0f,
               .visual_id =
                   gfx::renderer::make_visual_id(rand() % 3, 0, 0, 0, 0, 0, 0)};
  }
  std::cout << "We give some vals, size is " << vals.size() << std::endl;

  return vals;
}
