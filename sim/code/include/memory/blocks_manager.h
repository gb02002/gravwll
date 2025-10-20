#pragma once
#include "ds/storage/particleBlock.h"
#include "memory/blocks_arena.h"
#include <vector>

class BlockMemoryManager {
public:
  explicit BlockMemoryManager(uint N_body);
  ~BlockMemoryManager() = default;

  ParticleBlock *create_block(MortonKey key);
  void destroy_block(ParticleBlock *p_bl);

  ParticleBlock *get_block_data(uint inx);
  const ParticleBlock *get_block_data(uint inx) const;
  MortonKey get_block_key(uint inx) const;

  void swap_blocks(uint inx_a, uint inx_b);
  void compact();

  class Iterator;
  Iterator begin();
  Iterator end();

  uint get_capacity() const { return arena_.capacity; };
  uint get_used_blocks() const {
    return arena_.current_counter;
    ;
  }

private:
  static uint compute_capacity(uint N_body) {
    const uint estimated_blocks = (N_body / 16) * 2;
    const uint min_blocks = 10;

    return std::max(estimated_blocks, min_blocks);
  }
  BlocksAllocator arena_;
  std::vector<bool> active_blocks_;
  std::vector<MortonKey> block_keys_;
};

class BlockMemoryManager::Iterator {
public:
  Iterator(BlockMemoryManager *manager, uint index)
      : manager_(manager), current_index_(index) {}

  ParticleBlock &operator*() {
    return *manager_->get_block_data(current_index_);
  }
  ParticleBlock *operator->() {
    return manager_->get_block_data(current_index_);
  }
  Iterator &operator++() {
    do {
      current_index_++;
    } while (current_index_ < manager_->get_capacity() &&
             !manager_->active_blocks_[current_index_]);
    return *this;
  }
  bool operator!=(const Iterator &other) const {
    return current_index_ != other.current_index_;
  }

private:
  BlockMemoryManager *manager_;
  uint current_index_;
};
