#include "utils/namespaces/MyMath.h"
#include "gtest/gtest.h"
#include <cerrno>
#include <ostream>
#define private public
#include "ds/tree/octree.h"

// Функция для проверки принадлежности точки bounding box
bool pointInBB(const MyMath::Vector3 &pos, const MyMath::BoundingBox &bb) {
  bool inside = (bb.min.x <= pos.x && pos.x <= bb.max.x) &&
                (bb.min.y <= pos.y && pos.y <= bb.max.y) &&
                (bb.min.z <= pos.z && pos.z <= bb.max.z);
  if (!inside) {
    std::cerr << "Particle at (" << pos.x << ", " << pos.y << ", " << pos.z
              << ") is OUTSIDE bounding box: [(" << bb.min.x << ", " << bb.min.y
              << ", " << bb.min.z << ") - (" << bb.max.x << ", " << bb.max.y
              << ", " << bb.max.z << ")]\n";
  }
  return inside;
}

// Функция вычисления октантового индекса (аналог вашей реализации)
int computeChildIndex(const MyMath::Vector3 &p, const MyMath::Vector3 &center) {
  return ((p.x >= center.x) << 2) | ((p.y >= center.y) << 1) |
         (p.z >= center.z);
}

TEST(OctreeTest, InsertWithSplit_ExactCountsAndBBValidity_ByMass_NoDuplicates) {
  Storage storage{};
  AROctreeNode root{MyMath::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
                    Multipole{}, 0, 10, storage};

  // Для каждого октанта (0..7) генерируем 2 точки
  // Массы задаём последовательно:
  // Child 0: 1.0, 2.0
  // Child 1: 3.0, 4.0
  // Child 2: 5.0, 6.0
  // Child 3: 7.0, 8.0
  // Child 4: 9.0, 10.0
  // Child 5: 11.0, 12.0
  // Child 6: 13.0, 14.0
  // Child 7: 15.0, 16.0

  std::vector<Particle> particles = {
      {0.1, 0.1, 0.1, 0, 0, 0, 1.0}, // Октант 0 (low)
      {0.2, 0.2, 0.2, 0, 0, 0, 2.0}, // Октант 0 (high)

      {0.1, 0.1, 0.6, 0, 0, 0, 3.0}, // Октант 1 (low)
      {0.2, 0.2, 0.7, 0, 0, 0, 4.0}, // Октант 1 (high)

      {0.1, 0.6, 0.1, 0, 0, 0, 5.0}, // Октант 2 (low)
      {0.2, 0.7, 0.2, 0, 0, 0, 6.0}, // Октант 2 (high)

      {0.1, 0.6, 0.6, 0, 0, 0, 7.0}, // Октант 3 (low)
      {0.2, 0.7, 0.7, 0, 0, 0, 8.0}, // Октант 3 (high)

      {0.6, 0.1, 0.1, 0, 0, 0, 9.0},  // Октант 4 (low)
      {0.7, 0.2, 0.2, 0, 0, 0, 10.0}, // Октант 4 (high)

      {0.6, 0.1, 0.6, 0, 0, 0, 11.0}, // Октант 5 (low)
      {0.7, 0.2, 0.7, 0, 0, 0, 12.0}, // Октант 5 (high)

      {0.6, 0.6, 0.1, 0, 0, 0, 13.0}, // Октант 6 (low)
      {0.7, 0.7, 0.2, 0, 0, 0, 14.0}, // Октант 6 (high)

      {0.6, 0.6, 0.6, 0, 0, 0, 15.0}, // Октант 7 (low)
      {0.7, 0.7, 0.7, 0, 0, 0, 16.0}  // Октант 7 (high)
  };

  // Выводим рассчитанный октант для каждой частицы перед вставкой.
  for (const auto &p : particles) {
    int idx = root.getChildIndex(MyMath::Vector3{p.x, p.y, p.z});
    std::cout << "About to insert particle with mass " << p.mass << " at ("
              << p.x << ", " << p.y << ", " << p.z
              << ") -> computed octant: " << idx << "\n";
    root.insert(p);
  }

  root.printOctreeMasses();
  // Вставляем дополнительную частицу для Child 7.
  Particle extra{0.65, 0.65, 0.65, 0, 0, 0, 199.0};
  std::cout << "About to insert extra particle with mass " << extra.mass
            << " at (" << extra.x << ", " << extra.y << ", " << extra.z
            << ") -> computed octant: "
            << root.getChildIndex(MyMath::Vector3(extra.x, extra.y, extra.z))
            << "\n";
  root.insert(extra);

  // После вставки 17-й частицы должен произойти split:
  EXPECT_EQ(root.localBlock.get(), nullptr)
      << "Root localBlock should be null after split.";
  for (int i = 0; i < 8; i++) {
    EXPECT_NE(root.children[i], nullptr)
        << "Child " << i << " is not initialized.";
  }

  std::array<MyMath::BoundingBox, 8> expectedBB = {
      {// Child 0 (000)
       {{0.0, 0.0, 0.0}, {0.5, 0.5, 0.5}},
       // Child 1 (001)
       {{0.0, 0.0, 0.5}, {0.5, 0.5, 1.0}},
       // Child 2 (010)
       {{0.0, 0.5, 0.0}, {0.5, 1.0, 0.5}},
       // Child 3 (011)
       {{0.0, 0.5, 0.5}, {0.5, 1.0, 1.0}},
       // Child 4 (100)
       {{0.5, 0.0, 0.0}, {1.0, 0.5, 0.5}},
       // Child 5 (101)
       {{0.5, 0.0, 0.5}, {1.0, 0.5, 1.0}},
       // Child 6 (110)
       {{0.5, 0.5, 0.0}, {1.0, 1.0, 0.5}},
       // Child 7 (111)
       {{0.5, 0.5, 0.5}, {1.0, 1.0, 1.0}}}};

  for (int i = 0; i < 8; i++) {
    MyMath::BoundingBox childBB = root.children[i]->bounds;
    MyMath::BoundingBox expBB = expectedBB[i];
    EXPECT_DOUBLE_EQ(childBB.min.x, expBB.min.x) << "Child " << i << " min.x";
    EXPECT_DOUBLE_EQ(childBB.min.y, expBB.min.y) << "Child " << i << " min.y";
    EXPECT_DOUBLE_EQ(childBB.min.z, expBB.min.z) << "Child " << i << " min.z";
    EXPECT_DOUBLE_EQ(childBB.max.x, expBB.max.x) << "Child " << i << " max.x";
    EXPECT_DOUBLE_EQ(childBB.max.y, expBB.max.y) << "Child " << i << " max.y";
    EXPECT_DOUBLE_EQ(childBB.max.z, expBB.max.z) << "Child " << i << " max.z";
  }

  // Ожидаемые массы для каждого октанта:
  // Child 0: {1.0, 2.0}
  // Child 1: {3.0, 4.0}
  // Child 2: {5.0, 6.0}
  // Child 3: {7.0, 8.0}
  // Child 4: {9.0, 10.0}
  // Child 5: {11.0, 12.0}
  // Child 6: {13.0, 14.0}
  // Child 7: {15.0, 16.0, 199.0}
  std::array<std::set<double>, 8> expectedMasses = {
      std::set<double>{1.0, 2.0},   std::set<double>{3.0, 4.0},
      std::set<double>{5.0, 6.0},   std::set<double>{7.0, 8.0},
      std::set<double>{9.0, 10.0},  std::set<double>{11.0, 12.0},
      std::set<double>{13.0, 14.0}, std::set<double>{15.0, 16.0, 199.0}};

  int totalParticles = 0;
  for (int i = 0; i < 8; i++) {
    int count =
        (root.children[i]->localBlock ? root.children[i]->localBlock->size : 0);
    totalParticles += count;
    std::set<double> foundMasses;
    for (int j = 0; j < root.children[i]->localBlock->size; j++) {
      MyMath::Vector3 pos = root.children[i]->localBlock->getPosition(j);
      // Формируем строку с координатами для отладки без operator<< для
      // MyMath::Vector3:
      std::string posStr = "(" + std::to_string(pos.x) + ", " +
                           std::to_string(pos.y) + ", " +
                           std::to_string(pos.z) + ")";
      EXPECT_TRUE(pointInBB(pos, root.children[i]->bounds))
          << "Particle at " << posStr << " in child " << i
          << " is outside its bounding box.";
      double m = root.children[i]->localBlock->getParticle(j).mass;
      foundMasses.insert(m);
    }
    EXPECT_EQ(foundMasses, expectedMasses[i])
        << "Mismatch in expected masses for child " << i;
  }
  EXPECT_EQ(totalParticles, 17) << "Total particles in children must be 17.";
  root.printOctreeMasses();
  std::cout << root.children[0]->localBlock->size << std::endl;
  std::cout << root.children[0]->localBlock->getParticle(0) << std::endl;
  std::cout << root.children[0]->localBlock->getParticle(1) << std::endl;
}
