#include "engine/engine.h"
#include "ctx/ctx.h"
#include "ds/storage/storage.h"
#include "ds/tree/octree.h"
#include "engine/pairwise.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

void PhysicsEngine::MainCycle() {
  std::cout << "Мы в основном цикле движка!";
  using namespace std::chrono;

  // Начальное значение следующего тика
  auto nextTickTime = high_resolution_clock::now() + p_ctx.integration_step;

  while (state.is_running()) {
    auto startOfTick = high_resolution_clock::now();
    physicsTick(startOfTick);

    auto endOfTick = high_resolution_clock::now();
    microseconds tickDuration =
        duration_cast<microseconds>(endOfTick - startOfTick);

    // Если вычисление тика заняло больше времени, чем
    // IntegrationStepInMicroseconds, обновляем nextTickTime, чтобы не ждать, а
    // начать следующий тик сразу.
    if (tickDuration > p_ctx.integration_step) {
      // Можно также обновить интеграционный шаг, если требуется адаптивное
      // управление:
      p_ctx.integration_step = tickDuration;
      nextTickTime = endOfTick;
    } else {
      // Если времени ещё осталось, ждем до следующего тика
      std::this_thread::sleep_until(nextTickTime);
      nextTickTime += p_ctx.integration_step;
    }
  }
  return;
}

int countPoints(AROctreeNode *node) {
  if (!node)
    return 0;

  int count = 0;
  if (node->localBlock != nullptr)
    count += node->localBlock->data_block.size;

  // Рекурсивно обходим всех детей (предполагается, что children — массив из 8
  // элементов)
  for (int i = 0; i < 8; ++i) {
    count += countPoints(node->children[i]);
  }
  return count;
}

// Тут верхнеуровнево вызываем 2 этапа -> каждому треду по pairwise ->
// синхронизация -> расчет мультиполя -> треды считают воздействие поля на
// остельные частицы -> синхронизация
int PhysicsEngine::physicsTick(
    std::chrono::high_resolution_clock::time_point tickTime) {
  // Получаем корневой узел дерева
  AROctreeNode *root = tree->get_root();
  calcBlocskAx(*root->localBlock);
  updateCoords(*root->localBlock, p_ctx.integration_step);
  return 0;
}

PhysicsEngine::PhysicsEngine(PhysicsCtx &p_ctx, SimulationState &state,
                             Storage &storage, DataCtx &d_ctx)
    : p_ctx(p_ctx), d_ctx(d_ctx), state(state), storage(storage),
      tree(std::make_unique<AROctree>(p_ctx.tree_depth(), d_ctx.bounding_box_,
                                      storage)) {
  std::cout << "Engine got initialized!\n";
  tree->insert_batch(d_ctx.access_dataset());
};

void PhysicsEngine::Init() {
  // std::vector<Particle> initDataSet = {
  //     {0.15, 0.1, 0.1, 0, 0, 0, 5.972 * 10e3}, // Октант 0 (low)
  //     {0.9, 0.5, 0.9, 0, 0, 0, 6.39 * 10e4},   // Октант 7 (high)
  //
  //     // {0.1, 0.1, 0.6, 0, 0, 0, 3.0}, // Октант 1 (low)
  //     {0.2, 0.2, 0.7, 0, 0, 0, 40 * 10e3}, // Октант 1 (high)
  //     //
  //     // {0.1, 0.6, 0.1, 0, 0, 0, 5.0}, // Октант 2 (low)
  //     {0.2, 0.7, 0.2, 100, 0, 0, 2 * 10e4}, // Октант 2 (high)
  //                                           //
  //     // {0.1, 0.6, 0.6, 0, 0, 0, 7.0}, // Октант 3 (low)
  //     {0.2, 0.4, 0.9, 0, 0, 0, 2 * 10e4}, // Октант 3 (high)
  //     //
  //     {0.6, 0.1, 0.1, 0, 0, 0, 10e4}, // Октант 4 (low)
  //     // {0.7, 0.2, 0.2, 0, 0, 0, 10.0}, // Октант 4 (high)
  //     //
  //     {0.6, 0.1, 0.6, 0, 0, 0, 10e4}, // Октант 5 (low)
  //     // {0.7, 0.2, 0.7, 0, 0, 0, 12.0}, // Октант 5 (high)
  //     //
  //     // {0.6, 0.6, 0.1, 0, 0, 0, 13.0}, // Октант 6 (low)
  //     {0.7, 0.7, 0.2, 0, 0, 0, 1.0 * 10e4}, // Октант 6 (high)
  //                                           //
  //     {0.6, 0.6, 0.6, 0, 0, 0, 15.0},       // Октант 7 (low)
  //     // {0.7, 0.7, 0.7, 0, 0, 0, 16.0}, // Октант 7 (high)
  //     // {0.65, 0.65, 0.65, 0, 0, 0, 199.0}};
  // };
  InitThreads();

  std::thread PEthread(&PhysicsEngine::MainCycle, this);
  PEthread.detach();
}

void PhysicsEngine::InitThreads() {
  for (int i = 0; i < std::thread::hardware_concurrency() - 1; ++i)
    threads.push_back(std::thread());
};
