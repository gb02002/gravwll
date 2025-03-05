#include "engine/engine.h"
#include "ds/storage/storage.h"
#include "ds/tree/octree.h"
#include <iostream>
#include <memory>

// Главный цикл физики
int PhysicsEngine::genCycle() { return 0; };

int PhysicsEngine::physicsTick() { return 0; };

PhysicsEngine::PhysicsEngine(Cfx &cfx)
    : cfx(cfx), storage(Storage()),
      tree(std::make_unique<AROctree>(cfx, storage)) {
  std::cout << "engine got initialized!\n";
};
