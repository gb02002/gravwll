#pragma once
#include "ds/storage/particleBlock.h"

void calcBlocskAx(ParticleBlock &block);

void updateCoords(ParticleBlock &block, std::chrono::microseconds dt);
