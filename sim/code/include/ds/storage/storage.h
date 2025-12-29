#pragma once
#include "memory/blocks_manager.h"
#include "particleBlock.h"
#include <cstddef>
#include <vector>

class Storage {
public:
  Storage(uint N_body);

private:
  BlockMemoryManager manager_;

public:
  ParticleBlock *create_memory_block(uint morton_key,
                                     const std::vector<Particle> &particles);
  void release_block(ParticleBlock *block);
  void transferParticle(ParticleBlock *fromBlock, ParticleBlock *toBlock,
                        size_t index);
};
