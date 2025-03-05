#include "ds/storage/storage.h"
#include "ds/storage/particleBlock.h"
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>
#include <vector>

std::unique_ptr<ParticleBlock>
Storage::createMemBlock(const std::vector<Particle> &particles) {
  return std::make_unique<ParticleBlock>(particles);
}

void Storage::transferParticle(ParticleBlock *fromBlock, ParticleBlock *toBlock,
                               int index) {
  /*
  std::cout << "we at least arrived here\n";
  std::cout << "TransferParticle: fromBlock = " << fromBlock
            << ", toBlock = " << toBlock << "\n";
  std::cout << "fromBlock mutex = " << &(fromBlock->getMutex())
            << ", toBlock mutex = " << &(toBlock->getMutex()) << "\n";

  std::scoped_lock lock(fromBlock->getMutex(), toBlock->getMutex());
  std::cout << "we locked our mutexes";
  */

  std::cout << "TransferParticle: fromBlock = " << fromBlock
            << ", toBlock = " << toBlock << "\n";
  auto &mutex1 = fromBlock->getMutex();
  auto &mutex2 = toBlock->getMutex();
  std::cout << "fromBlock mutex = " << &mutex1
            << ", toBlock mutex = " << &mutex2 << "\n";

  // Попытка захвата обоих мьютексов
  while (true) {
    int lockResult = std::try_lock(mutex1, mutex2);
    if (lockResult == -1) { // -1 означает, что оба захвачены
      break;
    } else {
      std::cout << "Не удалось захватить mutex " << lockResult
                << ", повторяем попытку...\n";
      std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
  }

  // Теперь оба мьютекса захвачены; adopt_lock используется, чтобы сообщить
  // lock_guard, что мьютекс уже захвачен.
  std::lock_guard<std::mutex> lock1(mutex1, std::adopt_lock);
  std::lock_guard<std::mutex> lock2(mutex2, std::adopt_lock);

  std::cout << "we locked our mutexes\n";

  // Читаем значения из fromBlock
  double x = fromBlock->get_x()[index];
  double y = fromBlock->get_y()[index];
  double z = fromBlock->get_z()[index];
  double vx = fromBlock->get_vx()[index];
  double vy = fromBlock->get_vy()[index];
  double vz = fromBlock->get_vz()[index];
  double fx = fromBlock->get_fx()[index];
  double fy = fromBlock->get_fy()[index];
  double fz = fromBlock->get_fz()[index];
  double ax = fromBlock->get_ax()[index];
  double ay = fromBlock->get_ay()[index];
  double az = fromBlock->get_az()[index];
  double mass = fromBlock->get_mass()[index];

  // Создаем конкретную частицу, используя конструктор
  Particle newParticle(x, y, z, vx, vy, vz, fx, fy, fz, ax, ay, az, mass);

  std::cout << "We created P!\n";
  // Добавляем частицу в целевой блок.
  toBlock->addParticle(newParticle);

  // Удаляем частицу из исходного блока.
  fromBlock->deleteParticle(index);
}

Storage::Storage() { std::cout << "Storage got initialized!\n"; }
