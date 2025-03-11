#include "ds/storage/storage.h"
#include "gtest/gtest.h"
#define private public
#include "ds/tree/octree.h"

TEST(OctreeNodeTest, CheckChildBounds) {
  Storage storage{};
  // Создаем родительский узел с границами от (0,0,0) до (1,1,1)
  AROctreeNode parentNode{MyMath::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
                          Multipole{}, 0, 10, storage};
  // Вычисляем центр
  parentNode.setCalculatedCenter();
  MyMath::Vector3 expectedCenter{0.5, 0.5, 0.5};

  EXPECT_DOUBLE_EQ(parentNode.center.x, expectedCenter.x);
  EXPECT_DOUBLE_EQ(parentNode.center.y, expectedCenter.y);
  EXPECT_DOUBLE_EQ(parentNode.center.z, expectedCenter.z);

  // Вычисляем границы для детей
  auto childrenBoxes = parentNode.childBounds();
  std::array<MyMath::BoundingBox, 8> expectedBox = {
      {{{0.0, 0.0, 0.0}, {0.5, 0.5, 0.5}},
       {{0.0, 0.0, 0.5}, {0.5, 0.5, 1.0}},
       {{0.0, 0.5, 0.0}, {0.5, 1.0, 0.5}},
       {{0.0, 0.5, 0.5}, {0.5, 1.0, 1.0}},
       {{0.5, 0.0, 0.0}, {1.0, 0.5, 0.5}},
       {{0.5, 0.0, 0.5}, {1.0, 0.5, 1.0}},
       {{0.5, 0.5, 0.0}, {1.0, 1.0, 0.5}},
       {{0.5, 0.5, 0.5}, {1.0, 1.0, 1.0}}}};
  // Проверяем для каждого ребенка (0-7)
  for (int i = 0; i < 8; ++i) {
    MyMath::BoundingBox expBox = expectedBox[i];
    MyMath::BoundingBox childBox = childrenBoxes[i];

    EXPECT_DOUBLE_EQ(childBox.min.x, expBox.min.x) << "Child " << i << " min.x";
    EXPECT_DOUBLE_EQ(childBox.min.y, expBox.min.y) << "Child " << i << " min.y";
    EXPECT_DOUBLE_EQ(childBox.min.z, expBox.min.z) << "Child " << i << " min.z";
    EXPECT_DOUBLE_EQ(childBox.max.x, expBox.max.x) << "Child " << i << " max.x";
    EXPECT_DOUBLE_EQ(childBox.max.y, expBox.max.y) << "Child " << i << " max.y";
    EXPECT_DOUBLE_EQ(childBox.max.z, expBox.max.z) << "Child " << i << " max.z";
  }
}

TEST(OctreeNodeTest, CompareChildIndexFunctions) {
  // Создадим узел с bounding box [0,1]^3, где центр = {0.5, 0.5, 0.5}
  Storage storage{};
  AROctreeNode node{MyMath::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
                    Multipole{}, 0, 10, storage};

  // Точка, полностью внутри нижнего октанта (все координаты меньше центра)
  MyMath::Vector3 p_inside{0.2, 0.2, 0.2};
  EXPECT_EQ(node.getChildIndex(p_inside), 0);
  EXPECT_EQ(node.boundsCheck(p_inside), 0);

  // Точка, полностью внутри верхнего октанта (все координаты больше центра)
  MyMath::Vector3 p_upper{0.7, 0.7, 0.7};
  EXPECT_EQ(node.getChildIndex(p_upper), 7);
  EXPECT_EQ(node.boundsCheck(p_upper), 7);

  // Точка, где координата x равна центру:
  // getChildIndex считает её принадлежащей правой половине (так как >=), а
  // boundsCheck – левой (так как >)
  MyMath::Vector3 p_onBoundary{0.5, 0.2, 0.2};
  EXPECT_EQ(node.getChildIndex(p_onBoundary), 4)
      << "Ожидается, что при p.x == center.x функция getChildIndex вернёт "
         "октант 4";
  EXPECT_EQ(node.boundsCheck(p_onBoundary), 0)
      << "Ожидается, что при p.x == center.x функция boundsCheck вернёт октант "
         "0";
}

// Тест для проверки корректности распределения частиц по октантам после вставки
// и сплита
TEST(OctreeNodeTest, CheckCorrectnessOfDistribution) {
  Storage storage{};
  // Создаем родительский узел с bounding box от {0,0,0} до {1,1,1}
  AROctreeNode parentNode{MyMath::BoundingBox{{0.0, 0.0, 0.0}, {1.0, 1.0, 1.0}},
                          Multipole{}, 0, 10, storage};

  // Подготавливаем список частиц, чьи координаты расположены так, чтобы попасть
  // в разные октанты. Предполагается, что центр узла равен {0.5, 0.5, 0.5}.
  std::vector<Particle> particles = {
      Particle{0.2, 0.2, 0.2, 0, 0, 0, 1}, // октант 0: x,y,z < 0.5
      Particle{0.2, 0.2, 0.7, 0, 0, 0, 1}, // октант 1: x,y < 0.5, z >= 0.5
      Particle{0.2, 0.7, 0.2, 0, 0, 0,
               1}, // октант 2: x < 0.5, y >= 0.5, z < 0.5
      Particle{0.2, 0.7, 0.7, 0, 0, 0, 1}, // октант 3: x < 0.5, y,z >= 0.5
      Particle{0.7, 0.2, 0.2, 0, 0, 0, 1}, // октант 4: x >= 0.5, y,z < 0.5
      Particle{0.7, 0.2, 0.7, 0, 0, 0,
               1}, // октант 5: x >= 0.5, y < 0.5, z >= 0.5
      Particle{0.7, 0.7, 0.2, 0, 0, 0, 1}, // октант 6: x,z < 0.5, y >= 0.5  (но
                                           // тут x >= 0.5, y >= 0.5, z < 0.5)
  };
  //
  // Вызываем split, который распределяет частицы из родительского узла в
  // дочерние В данном примере split может принимать параметр (например, первую
  // частицу) для распределения. Если ваша реализация split отличается, вызовите
  // его согласно спецификации.
  parentNode.split(Particle{0.7, 0.7, 0.7, 0, 0, 0, 1} // октант 7: x,y,z >= 0.5
  ); // пример вызова с использованием данных первой частицы

  // Вставляем все частицы в родительский узел.
  // Предполагается, что метод insert добавляет частицу в локальный блок,
  // а при достижении определенного порога происходит вызов split для
  // распределения.
  for (const auto &p : particles) {
    parentNode.insert(p);
  }

  // Проверяем, что в каждом из 8 дочерних узлов оказалось ровно по одной
  // частице
  for (int i = 0; i < 8; ++i) {
    EXPECT_EQ(parentNode.children[i]->localBlock->size, 1)
        << "Неверное распределение частиц в октанте " << i;
  }
}
