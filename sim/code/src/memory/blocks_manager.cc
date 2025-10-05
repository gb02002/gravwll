#include "memory/blocks_manager.h"
#include "ds/storage/particleBlock.h"
#include "memory/blocks_arena.h"
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <utility>

BlockMemoryManager::BlockMemoryManager(uint N_body)
    : arena_(compute_capacity(N_body)) {

  if (arena_.initialize() != 0) {
    throw std::runtime_error("Failed to init arena");
  }

  active_blocks_.resize(arena_.capacity, false);
  block_keys_.resize(arena_.capacity);
}

ParticleBlock *BlockMemoryManager::create_block(MortonKey key) {
  ParticleBlock *block = arena_.allocate();
  if (!block)
    return nullptr;

  uint index =
      (reinterpret_cast<std::byte *>(block) - arena_.base) / arena_.block_size;
  block->initialize();
  block_keys_[index] = key;
  active_blocks_[index] = true;

  return block;
}

void BlockMemoryManager::destroy_block(ParticleBlock *p_bl) {
  if (!p_bl)
    return;

  uint index =
      (reinterpret_cast<std::byte *>(p_bl) - arena_.base) / arena_.block_size;
  arena_.deallocate(p_bl);
  active_blocks_[index] = false;
}

ParticleBlock *BlockMemoryManager::get_block_data(uint inx) {
  if (inx >= arena_.capacity || !active_blocks_[inx]) {
    return nullptr;
  }
  return reinterpret_cast<ParticleBlock *>(arena_.base +
                                           inx * arena_.block_size);
}

const ParticleBlock *BlockMemoryManager::get_block_data(uint index) const {
  if (index >= arena_.capacity || !active_blocks_[index]) {
    return nullptr;
  }
  return reinterpret_cast<const ParticleBlock *>(arena_.base +
                                                 index * arena_.block_size);
}

MortonKey BlockMemoryManager::get_block_key(uint inx) const {
  if (inx >= block_keys_.size()) {
    return MortonKey{};
  }
  return block_keys_[inx];
}

void BlockMemoryManager::swap_blocks(uint inx_a, uint inx_b) {
  if (inx_a >= arena_.capacity || inx_b >= arena_.capacity ||
      !active_blocks_[inx_a] || !active_blocks_[inx_b])
    return;

  ParticleBlock *block_a = get_block_data(inx_a);
  ParticleBlock *block_b = get_block_data(inx_b);

  if (!block_a || !block_b)
    return;

  block_a->swap(*block_b);

  std::swap(block_keys_[inx_a], block_keys_[inx_b]);
}

void BlockMemoryManager::compact() {
  uint free_slot = 0;
  while (free_slot < arena_.capacity && active_blocks_[free_slot]) {
    free_slot++;
  }

  for (uint i = free_slot + 1; i < arena_.capacity; i++) {
    if (active_blocks_[i]) {
      ParticleBlock *src_block = get_block_data(i);
      ParticleBlock *dest_block = get_block_data(free_slot);

      if (src_block && dest_block) {
        *dest_block = std::move(*src_block);
        block_keys_[free_slot] = block_keys_[i];
        active_blocks_[free_slot] = true;
        active_blocks_[i] = false;

        arena_.deallocate(src_block);
      }
      while (free_slot < arena_.capacity && active_blocks_[free_slot]) {
        free_slot++;
      }
    }
  }
}

BlockMemoryManager::Iterator BlockMemoryManager::begin() {
  uint first_index = 0;
  while (first_index < arena_.capacity && !active_blocks_[first_index]) {
    first_index++;
  }
  return Iterator(this, first_index);
}

BlockMemoryManager::Iterator BlockMemoryManager::end() {
  return Iterator(this, arena_.capacity);
}
