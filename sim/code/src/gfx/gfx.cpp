#include "gfx/gfx.h"
#include "ctx/ctx.h"
#include "ds/tree/octree.h"
#include "raymath.h"
#include <memory>
#include <raylib.h>

GfxEngine::GfxEngine(Ctx &ctx, AROctree &tree) : ctx(ctx), tree(tree) {};

void GfxEngine::Init() { Cam = InitRenderer(); }

void GfxEngine::Tick() {
  if (WindowShouldClose()) {
    ctx.state().request_exit();
    return;
  }

  this->CheckKeys();
  BeginDrawing();
  ClearBackground(BLACK);
  BeginMode3D(Cam->camera);

  StartRenderTree();

  EndMode3D();
  EndDrawing();
}

void GfxEngine::CleanUp() { CloseWindow(); }

void GfxEngine::CheckKeys() {
  if (IsKeyDown(KEY_LEFT) || IsKeyDown(KEY_RIGHT) || IsKeyDown(KEY_UP) ||
      IsKeyDown(KEY_DOWN)) {
    UpdateCameraManual(40.0f);
  }
  if (IsKeyDown(KEY_Q) || IsKeyDown(KEY_E)) {
    UpdateCameraVerticalMovement(1.0f);
  }

  if (IsKeyDown(KEY_W) || IsKeyDown(KEY_A) || IsKeyDown(KEY_S) ||
      IsKeyDown(KEY_D)) {
    UpdateCameraPosition(1.5f);
  }
};

void GfxEngine::UpdateCameraManual(float rotationSpeedDegrees) {
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

  Cam->yaw += yawChange;
  Cam->pitch =
      fmaxf(fminf(Cam->pitch + pitchChange, DEG2RAD * 89.0f), -DEG2RAD * 89.0f);

  // Пересчитываем направление
  Vector3 direction = {cosf(Cam->yaw) * cosf(Cam->pitch), sinf(Cam->pitch),
                       sinf(Cam->yaw) * cosf(Cam->pitch)};

  direction = Vector3Normalize(direction);

  // Передаем изменения в Camera3D
  Cam->camera.target = Vector3Add(Cam->camera.position, direction);
};

void GfxEngine::UpdateCameraRotation(float rotationSpeedDegrees) {
  float deltaAngle = rotationSpeedDegrees * DEG2RAD * GetFrameTime();
  Vector3 dir = Vector3Subtract(Cam->position, Cam->target);
  float radius = sqrtf(dir.x * dir.x + dir.z * dir.z);
  float currentAngle = atan2f(dir.z, dir.x);
  currentAngle += deltaAngle;
  Cam->position.x = Cam->target.x + radius * cosf(currentAngle);
  Cam->position.z = Cam->target.z + radius * sinf(currentAngle);
};

void GfxEngine::UpdateCameraPosition(float moveSpeed) {
  float deltaTime = GetFrameTime();
  Vector3 forward = Vector3Normalize(
      Vector3Subtract(Cam->camera.target, Cam->camera.position));
  Vector3 right =
      Vector3Normalize(Vector3CrossProduct(forward, {0.0f, 1.0f, 0.0f}));

  Vector3 movement = {0.0f, 0.0f, 0.0f};
  if (IsKeyDown(KEY_W)) {
    movement = Vector3Add(movement, forward);
  }
  if (IsKeyDown(KEY_S)) {
    movement = Vector3Subtract(movement, forward);
  }
  if (IsKeyDown(KEY_A))
    movement = Vector3Subtract(movement, right);
  if (IsKeyDown(KEY_D))
    movement = Vector3Add(movement, right);

  if (Vector3Length(movement) > 0.0f) {
    movement = Vector3Scale(Vector3Normalize(movement), moveSpeed * deltaTime);
    Cam->camera.position = Vector3Add(Cam->camera.position, movement);
    Cam->camera.target = Vector3Add(Cam->camera.target, movement);
  }
};

void GfxEngine::UpdateCameraVerticalMovement(float moveSpeed) {
  float deltaTime = GetFrameTime();
  auto deltaPositionScalar = moveSpeed * deltaTime;
  if (IsKeyDown(KEY_Q))
    deltaPositionScalar = -deltaPositionScalar;

  // Вычисляем направление взгляда (forward) и нормализуем его
  auto forward = Vector3Normalize(
      Vector3Subtract(Cam->camera.target, Cam->camera.position));

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
  Cam->camera.position = Vector3Add(Cam->camera.position, deltaPosition);
  Cam->camera.target = Vector3Add(Cam->camera.target, deltaPosition);
};

std::unique_ptr<MyCamera3D> GfxEngine::InitRenderer() {
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

void GfxEngine::StartRenderTree() {
  MyMath::Vector3 baseMin{0.0, 0.0, 0.0}, baseMax{1.0, 1.0, 1.0};
  auto baseCenter = (baseMin + baseMax) * 0.5;
  auto baseSize = baseMax - baseMin;

  DrawCubeWires({static_cast<float>(baseCenter.x),
                 static_cast<float>(baseCenter.y),
                 static_cast<float>(baseCenter.z)},
                static_cast<float>(baseSize.x), static_cast<float>(baseSize.y),
                static_cast<float>(baseSize.z), GREEN);

  RenderNode(tree.get_root());
}

void GfxEngine::RenderNode(AROctreeNode *node) {
  if (!node)
    return;
  // std::lock_guard<std::mutex> lock(node->getMutex());

  if (node->localBlock != nullptr) {
    auto nodeCenter = (node->bounds.min + node->bounds.max) * 0.5;
    auto nodeSize = node->bounds.max - node->bounds.min;
    //
    // std::cout << "nodeCenter: " << nodeCenter.x << nodeCenter.y <<
    // nodeCenter.z
    //           << std::endl;
    // std::cout << "nodeSize: " << nodeSize.x << nodeSize.y << nodeSize.z
    //           << std::endl;

    DrawCubeWires(
        {static_cast<float>(nodeCenter.x), static_cast<float>(nodeCenter.y),
         static_cast<float>(nodeCenter.z)},
        static_cast<float>(nodeSize.x), static_cast<float>(nodeSize.y),
        static_cast<float>(nodeSize.z), RED);

    for (int i = 0; i < node->localBlock->data_block.size; ++i) {
      auto pos = node->localBlock->data_block.getPosition(i);
      DrawCube({static_cast<float>(pos.x), static_cast<float>(pos.y),
                static_cast<float>(pos.z)},
               0.005f, 0.005f, 0.005f, WHITE);
    }
  }

  for (auto &child : node->children) {
    if (child)
      RenderNode(child);
  }
};
