#include "ds/storage/storage.h"
#include "ds/storage/particleBlock.h"
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

Storage::Storage(uint N_body) : manager_(N_body) {
  std::cout << "Storage got initialized!\n";
}

ParticleBlock *
Storage::create_memory_block(uint morton_key,
                             const std::vector<Particle> &particles) {
  ParticleBlock *block_address = manager_.create_block(MortonKey{morton_key});
  new (block_address) ParticleBlock(morton_key, particles);
  return block_address;
}

void Storage::release_block(ParticleBlock *block) {
  manager_.destroy_block(block);
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
  auto &mutex1 = fromBlock->data_block.getMutex();
  auto &mutex2 = toBlock->data_block.getMutex();
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
  double x = fromBlock->data_block.get_x()[index];
  double y = fromBlock->data_block.get_y()[index];
  double z = fromBlock->data_block.get_z()[index];
  double vx = fromBlock->data_block.get_vx()[index];
  double vy = fromBlock->data_block.get_vy()[index];
  double vz = fromBlock->data_block.get_vz()[index];
  double fx = fromBlock->data_block.get_fx()[index];
  double fy = fromBlock->data_block.get_fy()[index];
  double fz = fromBlock->data_block.get_fz()[index];
  double ax = fromBlock->data_block.get_ax()[index];
  double ay = fromBlock->data_block.get_ay()[index];
  double az = fromBlock->data_block.get_az()[index];
  double mass = fromBlock->data_block.get_mass()[index];

  // Создаем конкретную частицу, используя конструктор
  Particle newParticle(x, y, z, vx, vy, vz, fx, fy, fz, ax, ay, az, mass);

  std::cout << "We created P!\n";
  // Добавляем частицу в целевой блок.
  toBlock->data_block.addParticle(newParticle);

  // Удаляем частицу из исходного блока.
  fromBlock->data_block.deleteParticle(index);
}
