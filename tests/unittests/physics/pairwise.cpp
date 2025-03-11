#include "engine/pairwise.h"
#include "core/bodies/particles.h"
#include "ds/storage/storage.h"
#include "gtest/gtest.h"
#include <memory>
#include <vector>
#define private public
#include "ds/storage/particleBlock.h"

TEST(PairWiseTests, TickComplitness) {
  std::vector<Particle> initial_data = {
      Particle{1, 1, 1, 0, 0, 0, 1},
      Particle{2, 2, 2, 0, 0, 0, 2},
      Particle{3, 3, 3, 0, 0, 0, 3},
  };
  Storage storage{};
  std::unique_ptr<ParticleBlock> testBlock =
      storage.createMemBlock(initial_data);

  calcBlocskAX(*testBlock);

  EXPECT_NE(testBlock->get_ax()[0], initial_data[0].getAx());
};
