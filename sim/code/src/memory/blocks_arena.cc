#include "memory/blocks_arena.h"
#include "ds/storage/particleBlock.h"
#include <cstddef>
#include <cstdlib>

ParticleBlock *BlocksAllocator::allocate() {
  if (free_head >= capacity)
    return nullptr;

  uint index = free_head;
  free_head = next_free_array[free_head];
  current_counter++;

  std::byte *block_ptr = base + index * block_size;
  return reinterpret_cast<ParticleBlock *>(block_ptr);
}

void BlocksAllocator::deallocate(ParticleBlock *block) {
  if (!block)
    return;
  uint index = (reinterpret_cast<std::byte *>(block) - base) / block_size;

  if (index >= capacity)
    return;

  next_free_array[index] = free_head;
  free_head = index;
  current_counter--;
}

int BlocksAllocator::initialize() {
  try {
    base = static_cast<std::byte *>(malloc(capacity * block_size));
    if (base == nullptr)
      return -1;
    last_block = base + (capacity - 1) * capacity;
    return 0;
  } catch (...) {
    return -1;
  }
}

// // 0 if good, -1 if error
// int BlocksAllocator::initialize() {
//   try {
//     this->base = (std::byte *)malloc(capacity * block_size);
//   } catch (std::runtime_error) {
//     std::cout << "Can't allocate memory";
//     return -1;
//   }
//   if (this->base == nullptr) {
//     return -1;
//   }
//   return 0;
// };

// Separate func, as we might get out of range and need reallocate without
// destroying everything
void BlocksAllocator::clean_up() {
  if (base != nullptr) {
    free(base);
    base = nullptr;
    last_block = nullptr;
  };
};

BlocksAllocator::~BlocksAllocator() { clean_up(); };
