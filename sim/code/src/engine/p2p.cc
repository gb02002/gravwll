#include "ds/storage/particleBlock.h"
#include "ds/tree/octree.h"
#include "engine/engine.h"
#include <oneapi/tbb/parallel_for.h>
#include <vector>

namespace operands::p2p {

void compute_p2p(ParticleBlock::DataBlock &block1,
                 ParticleBlock::DataBlock &block2) {}

void compute_p2p_with_itself(ParticleBlock::DataBlock &block1) {}
void apply_force(ParticleBlock::DataBlock &block1) {}

// TODO:
// 1. How to create vector?
void compute_direct_interactions(std::vector<ParticleBlock *> leaves) {
  tbb::parallel_for(
      tbb::blocked_range<size_t>(0, leaves.size()),
      [&](const auto &r) {
        for (size_t i = r.begin(); i < r.end(); ++i) {
          ParticleBlock *leaf = leaves[i];

          for (ParticleBlock *neighbor : leaf->near_field) {
            if (neighbor)
              compute_p2p(leaf->data_block, neighbor->data_block);
          }
          compute_p2p_with_itself(leaf->data_block);
        }
      },
      tbb::auto_partitioner());
  tbb::parallel_for(
      tbb::blocked_range<size_t>(0, leaves.size()),
      [&](const auto &r) {
        for (size_t i = r.begin(); i < r.end(); ++i) {
          ParticleBlock *leaf = leaves[i];
          apply_force(leaf->data_block);
        }
      },
      tbb::auto_partitioner());
}

std::vector<ParticleBlock *> get_all_leaves(AROctree *tree) {
  AROctreeNode *root = tree->get_root();
  return {};
}
} // namespace operands::p2p

// NOTE: Simple recursion with deref
void PhysicsEngine::run() {
  debug::debug_print("p2p start...");
  operands::p2p::compute_direct_interactions(
      operands::p2p::get_all_leaves(tree.get()));
}
