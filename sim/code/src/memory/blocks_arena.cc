#define CURRENT_MODULE_DEBUG 0
#include "memory/blocks_arena.h"
#include "ds/storage/particleBlock.h"
#include "utils/namespaces/error_namespace.h"
#include <cstddef>
#include <cstdlib>

ParticleBlock *BlocksAllocator::allocate() {
  debug::debug_print("We entered BlocksAllocator::allocate");
  if (free_head >= capacity) {
    debug::debug_print("free_head is bigger or equal to capacity: {} >= {}",
                       free_head, capacity);
    return nullptr;
  }
  size_t index = free_head;
  free_head = next_free_array[free_head];
  current_counter++;

  std::byte *block_ptr = base + index * k_block_size;
  debug::debug_print("current_counter: {}", current_counter);
  return reinterpret_cast<ParticleBlock *>(block_ptr);
}

void BlocksAllocator::deallocate(ParticleBlock *block) {
  if (!block)
    return;
  size_t index =
      static_cast<size_t>((reinterpret_cast<std::byte *>(block) - base)) /
      k_block_size;

  if (index >= capacity)
    return;

  next_free_array[index] = free_head;
  free_head = index;
  current_counter--;
}

int BlocksAllocator::initialize() {
  try {
    base = static_cast<std::byte *>(malloc(capacity * k_block_size));
    if (base == nullptr)
      return -1;
    last_block = base + (capacity - 1) * capacity;
    return 0;
  } catch (...) {
    return -1;
  }
}

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
