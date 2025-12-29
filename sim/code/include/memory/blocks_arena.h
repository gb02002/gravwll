#pragma once
#include "ds/storage/particleBlock.h"
#include <cstddef>
#include <vector>
/* This file contains memory arena used by BlockMemnoryManager */

class BlocksAllocator {
public:
  BlocksAllocator(size_t capacity)
      : k_block_size(sizeof(ParticleBlock)), capacity(capacity),
        current_counter(0), free_head(0), next_free_array(capacity) {
    if (capacity > 0) {
      for (uint i = 0; i < capacity; ++i) {
        next_free_array[i] = (i + 1);
      }
      next_free_array[capacity - 1] = capacity;
    }
  }
  ~BlocksAllocator();

  int initialize();
  void clean_up();

  ParticleBlock *allocate();
  void deallocate(ParticleBlock *block);

public:
  const size_t k_block_size = sizeof(ParticleBlock);
  size_t capacity;

  size_t current_counter = 0;
  size_t free_head;

  std::byte *base = nullptr;
  std::byte *last_block = nullptr;

private:
  std::vector<size_t> next_free_array; // forms the indexes with free blockss
};
