#include "gtest/gtest.h"
#define private public
#include "ds/tree/octree.h"

TEST(OctreeInsertTest, InsertWithoutSplit) {
  Storage storage{};
  AROctreeNode root{MyMath::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
                    Multipole{}, 0, 10, storage};

  // Вставляем 10 частиц (менее 16, порог для разбиения)
  const int numParticles = 10;
  for (int i = 0; i < numParticles; i++) {
    Particle p{0.1 + i * 0.01, 0.1, 0.1, 0, 0, 0, 1};
    root.insert(p);
  }

  // Проверяем, что все частицы добавлены в локальный блок, а разбиения не
  // произошло (children остаются nullptr)
  EXPECT_EQ(root.localBlock->size, numParticles);
  for (int i = 0; i < 8; i++) {
    EXPECT_EQ(root.children[i], nullptr);
  }
}
