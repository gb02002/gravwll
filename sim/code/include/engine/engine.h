#pragma once

#include "ds/tree/octree.h"
#include <memory>
#include <thread>
#include <vector>

class CloseInteractionElement {};

class MultipoleInteractionElement {};

class PhysicsEngine {
public:
  explicit PhysicsEngine(Cfx &cfx);

  ~PhysicsEngine() {
    for (int th_n = 0; th_n < threads.size(); ++th_n) {
      if (threads[th_n].joinable())
        threads[th_n].join();
    }
    return;
  };

  void MainCycle();
  void Init();

  std::unique_ptr<AROctree> tree;

private:
  Cfx &cfx;
  Storage storage;
  int physicsTick();

  std::vector<std::thread> threads;

  CloseInteractionElement closeInteraction;
  MultipoleInteractionElement multipoleInteraction;

  void InitThreads();
};
