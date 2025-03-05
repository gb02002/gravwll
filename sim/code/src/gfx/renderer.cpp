#include "gfx/renderer.h"
#include "ds/tree/octree.h"
#include "raylib.h"
#include "raymath.h"
#include <cstdio>
// #include <iostream>
#include <memory>

// Что-то делаем
void RenderNode(Camera3D *camera, AROctreeNode *node) {
  std::lock_guard<std::mutex> lock(node->getMutex());
  if (!node)
    return;

  if (node->localBlock != nullptr) {
    auto nodeCenter = (node->bounds.min + node->bounds.max) * 0.5;
    auto nodeSize = node->bounds.max - node->bounds.min;

    DrawCubeWires(
        {static_cast<float>(nodeCenter.x), static_cast<float>(nodeCenter.y),
         static_cast<float>(nodeCenter.z)},
        static_cast<float>(nodeSize.x), static_cast<float>(nodeSize.y),
        static_cast<float>(nodeSize.z), RED);

    for (int i = 0; i < node->localBlock->size; ++i) {
      auto pos = node->localBlock->getPosition(i);
      // DrawSphere({static_cast<float>(pos.x), static_cast<float>(pos.y),
      // static_cast<float>(pos.z)},
      // 0.001f, GRAY);
      DrawCube({static_cast<float>(pos.x), static_cast<float>(pos.y),
                static_cast<float>(pos.z)},
               0.001f, 0.001f, 0.001f, WHITE);
    }
  }

  for (auto &child : node->children) {
    if (child)
      RenderNode(camera, child);
  }
}

// Что-то делаем
std::unique_ptr<MyCamera3D> InitRenderer() {
  // const int screenWidth = 1000, screenHeight = 600;
  const int screenWidth = 2133, screenHeight = 1200;
  InitWindow(screenWidth, screenHeight, "Raylib: Камера и маленькие кубы");

  auto camera = std::make_unique<MyCamera3D>();
  camera->position = {2.0f, 2.0f, 2.0f};
  camera->target = {0.5f, 0.5f, 0.5f};
  camera->up = {0.0f, 1.0f, 0.0f};
  camera->camera.fovy = 45.0f;
  camera->camera.projection = CAMERA_PERSPECTIVE;
  return camera;
}

// Что-то делаем
void StartRenderTree(Camera3D *camera, AROctree *tree) {
  MyMath::Vector3 baseMin{0.0, 0.0, 0.0}, baseMax{1.0, 1.0, 1.0};
  auto baseCenter = (baseMin + baseMax) * 0.5;
  auto baseSize = baseMax - baseMin;

  DrawCubeWires({static_cast<float>(baseCenter.x),
                 static_cast<float>(baseCenter.y),
                 static_cast<float>(baseCenter.z)},
                static_cast<float>(baseSize.x), static_cast<float>(baseSize.y),
                static_cast<float>(baseSize.z), GREEN);

  if (tree)
    RenderNode(camera, tree->get_root());
}

// Что-то делаем
void CheckKeys(MyCamera3D *camera) {
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) ||
      IsKeyDown(KEY_DOWN)) {
    UpdateCameraManual(camera, 40.0f);
  }
  // } else {
  // UpdateCameraRotation(&camera->camera, 3.0f);
  // }

  if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_E)) {
    UpdateCameraVerticalMovement(camera, 1.0f);
  }

  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) ||
      IsKeyDown(KEY_D)) {
    UpdateCameraPosition(&camera->camera, 1.5f);
  }
}

// Что-то делаем
void UpdateCameraManual(MyCamera3D *camera, float rotationSpeedDegrees) {
  float deltaTime = GetFrameTime();
  float yawChange = 0.0f, pitchChange = 0.0f;

  if (IsKeyDown(KEY_RIGHT))
    yawChange += rotationSpeedDegrees * DEG2RAD * deltaTime;
  if (IsKeyDown(KEY_LEFT))
    yawChange -= rotationSpeedDegrees * DEG2RAD * deltaTime;
  if (IsKeyDown(KEY_UP))
    pitchChange += rotationSpeedDegrees * DEG2RAD * deltaTime;
  if (IsKeyDown(KEY_DOWN))
    pitchChange -= rotationSpeedDegrees * DEG2RAD * deltaTime;

  camera->yaw += yawChange;
  camera->pitch = fmaxf(fminf(camera->pitch + pitchChange, DEG2RAD * 89.0f),
                        -DEG2RAD * 89.0f);

  // Пересчитываем направление
  Vector3 direction = {cosf(camera->yaw) * cosf(camera->pitch),
                       sinf(camera->pitch),
                       sinf(camera->yaw) * cosf(camera->pitch)};

  direction = Vector3Normalize(direction);

  // Передаем изменения в Camera3D
  camera->camera.target = Vector3Add(camera->camera.position, direction);
}

void UpdateCameraVerticalMovement(MyCamera3D *camera, float moveSpeed) {
  float deltaTime = GetFrameTime();
  auto deltaPositionScalar = moveSpeed * deltaTime;
  if (IsKeyDown(KEY_Q))
    deltaPositionScalar = -deltaPositionScalar;

  // Вычисляем направление взгляда (forward) и нормализуем его
  auto forward = Vector3Normalize(
      Vector3Subtract(camera->camera.target, camera->camera.position));

  // Определяем мировой up, обычно (0, 1, 0)
  MyMath::Vector3 worldUp = MyMath::Vector3(0.0f, 1.0f, 0.0f);

  // Вычисляем правый вектор: нормализованное векторное произведение forward и
  // worldUp
  auto right = Vector3Normalize(Vector3CrossProduct(forward, worldUp));

  // Вычисляем локальный up: векторное произведение right и forward
  auto newUp = Vector3CrossProduct(right, forward);

  // Масштабируем новый up на величину смещения
  auto deltaPosition = Vector3Scale(newUp, deltaPositionScalar);

  // Сдвигаем как позицию, так и точку взгляда
  camera->camera.position = Vector3Add(camera->camera.position, deltaPosition);
  camera->camera.target = Vector3Add(camera->camera.target, deltaPosition);
}

// Что-то делаем
void UpdateCameraRotation(Camera3D *camera, float rotationSpeedDegrees) {
  float deltaAngle = rotationSpeedDegrees * DEG2RAD * GetFrameTime();
  Vector3 dir = Vector3Subtract(camera->position, camera->target);
  float radius = sqrtf(dir.x * dir.x + dir.z * dir.z);
  float currentAngle = atan2f(dir.z, dir.x);
  currentAngle += deltaAngle;
  camera->position.x = camera->target.x + radius * cosf(currentAngle);
  camera->position.z = camera->target.z + radius * sinf(currentAngle);
}

// Что-то делаем
void UpdateCameraPosition(Camera3D *camera, float moveSpeed) {
  float deltaTime = GetFrameTime();
  Vector3 forward =
      Vector3Normalize(Vector3Subtract(camera->target, camera->position));
  Vector3 right =
      Vector3Normalize(Vector3CrossProduct(forward, {0.0f, 1.0f, 0.0f}));

  Vector3 movement = {0.0f, 0.0f, 0.0f};
  if (IsKeyDown(KEY_W))
    movement = Vector3Add(movement, forward);
  if (IsKeyDown(KEY_S))
    movement = Vector3Subtract(movement, forward);
  if (IsKeyDown(KEY_A))
    movement = Vector3Subtract(movement, right);
  if (IsKeyDown(KEY_D))
    movement = Vector3Add(movement, right);

  if (Vector3Length(movement) > 0.0f) {
    movement = Vector3Scale(Vector3Normalize(movement), moveSpeed * deltaTime);
    camera->position = Vector3Add(camera->position, movement);
    camera->target = Vector3Add(camera->target, movement);
  }
}
