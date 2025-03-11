#include "engine/engine.h"
#include "ds/storage/storage.h"
#include "ds/tree/octree.h"
#include "engine/pairwise.h"
#include <chrono>
#include <iostream>
#include <memory>
#include <thread>
#include <vector>

// Главный цикл физики. Исполняется в своем потоке
void PhysicsEngine::MainCycle() {
  std::cout << "Мы в основном цикле движка!";
  using namespace std::chrono;

  auto nextTickTime =
      high_resolution_clock::now() + cfx.IntegrationStepInMicroseconds;

  while (cfx.state != EXIT) {
    auto startOfTick = high_resolution_clock::now();
    physicsTick(startOfTick);

    std::this_thread::sleep_until(nextTickTime);
    nextTickTime += cfx.IntegrationStepInMicroseconds;
    // Тут надо как-то добавить случай, если вычисление слишком долгое и нам не
    // приходится ждать. Надо вычислить, какое время заняло, и обновить
    // cfx.IntegrationStepInMicroseconds. Плюс, надо держать в голове второй
    // mode, где мы не имеем мин порог и стараемся его держаться, а всегда
    // оптимизируемся по максимуму.
  }

  return;
};

int countPoints(AROctreeNode *node) {
  if (!node)
    return 0;

  int count = 0;
  if (node->localBlock != nullptr)
    count += node->localBlock->size;

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
  updateCoords(*root->localBlock, cfx.IntegrationStepInMicroseconds);
  return 0;
}

PhysicsEngine::PhysicsEngine(Cfx &cfx)
    : cfx(cfx), storage(Storage()),
      tree(std::make_unique<AROctree>(cfx, storage)) {
  std::cout << "Engine got initialized!\n";
};

void PhysicsEngine::Init() {
  // auto initDataSet = cfx.CreateDataSet();
  std::vector<Particle> initDataSet = {
      {0.15, 0.1, 0.1, 0, 0, 0, 5.972 * 10e3}, // Октант 0 (low)
      {0.9, 0.5, 0.9, 0, 0, 0, 6.39 * 10e4},   // Октант 7 (high)

      // {0.1, 0.1, 0.6, 0, 0, 0, 3.0}, // Октант 1 (low)
      {0.2, 0.2, 0.7, 0, 0, 0, 40 * 10e3}, // Октант 1 (high)
      //
      // {0.1, 0.6, 0.1, 0, 0, 0, 5.0}, // Октант 2 (low)
      {0.2, 0.7, 0.2, 100, 0, 0, 3 * 10e4}, // Октант 2 (high)
                                            //
      // {0.1, 0.6, 0.6, 0, 0, 0, 7.0}, // Октант 3 (low)
      {0.2, 0.4, 0.7, 0, 0, 0, 3 * 10e4}, // Октант 3 (high)
      //
      {0.6, 0.1, 0.1, 0, 0, 0, 10e4}, // Октант 4 (low)
      // {0.7, 0.2, 0.2, 0, 0, 0, 10.0}, // Октант 4 (high)
      //
      {0.6, 0.1, 0.6, 0, 0, 0, 10e4}, // Октант 5 (low)
      // {0.7, 0.2, 0.7, 0, 0, 0, 12.0}, // Октант 5 (high)
      //
      // {0.6, 0.6, 0.1, 0, 0, 0, 13.0}, // Октант 6 (low)
      {0.7, 0.7, 0.2, 0, 0, 0, 1.0 * 10e4}, // Октант 6 (high)
                                            //
      {0.6, 0.6, 0.6, 0, 0, 0, 15.0},       // Октант 7 (low)
      // {0.7, 0.7, 0.7, 0, 0, 0, 16.0}, // Октант 7 (high)
      // {0.65, 0.65, 0.65, 0, 0, 0, 199.0}};
  };
  tree->insert_batch(initDataSet);
  InitThreads();

  std::thread PEthread(&PhysicsEngine::MainCycle, this);
  PEthread.detach();
}

void PhysicsEngine::InitThreads() {
  for (int i = 0; i < std::thread::hardware_concurrency() - 1; ++i)
    threads.push_back(std::thread());
};
