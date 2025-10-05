#pragma once
#include "ds/storage/particleBlock.h"
#include <cstddef>
#include <vector>
/* This file contains memory arena used by BlockMemnoryManager */

class BlocksAllocator {
public:
  BlocksAllocator(uint capacity)
      : capacity(capacity), block_size(sizeof(ParticleBlock)),
        next_free_array(capacity), free_head(0), current_counter(0) {
    if (capacity > 0) {
      for (uint i = 0; i < capacity; ++i) {
        next_free_array[i] = static_cast<int>(i + 1);
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
  std::byte *base = nullptr;
  std::byte *last_block = nullptr;
  uint current_counter = 0;
  uint free_head;

  uint capacity;
  size_t block_size = sizeof(ParticleBlock);

private:
  std::vector<int> next_free_array; // forms the indexes with free blockss
};
