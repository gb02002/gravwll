#pragma once
#include "particleBlock.h"
#include <memory>
#include <vector>

class Storage {
public:
  Storage();
  ~Storage() = default;

  std::unique_ptr<ParticleBlock>
  createMemBlock(const std::vector<Particle> &particles);
  void transferParticle(ParticleBlock *fromBlock, ParticleBlock *toBlock,
                        int index);
};
