#pragma once

#include "ctx/ctx.h"
#include "ctx/simulation_state.h"
#include "ds/tree/octree.h"
#include <memory>
#include <thread>
#include <vector>

class CloseInteractionElement {};

class MultipoleInteractionElement {};

class PhysicsEngine {
public:
  explicit PhysicsEngine(PhysicsCtx &p_ctx, SimulationState &state,
                         Storage &storage, DataCtx &d_ctx);

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
  PhysicsCtx &p_ctx;
  DataCtx &d_ctx;
  SimulationState &state;
  Storage &storage;
  int physicsTick(std::chrono::high_resolution_clock::time_point tickTime);

  std::vector<std::thread> threads;

  CloseInteractionElement closeInteraction;
  MultipoleInteractionElement multipoleInteraction;

  void init_threads();
};
