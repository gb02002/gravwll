#pragma once
#include "cfx/cfx.h"
#include "ds/tree/octree.h"
#include "gfx/camera.h"

class GfxEngine {
public:
  explicit GfxEngine(Cfx &cfx, AROctree &tree);

  void Init();
  void Tick();
  void Run();
  void CleanUp();

private:
  Cfx &cfx;
  AROctree &tree;
  std::unique_ptr<MyCamera3D> Cam;
  std::unique_ptr<MyCamera3D> InitRenderer();

  void CheckKeys();
  void UpdateCameraManual(float rotationSpeedDegrees);
  void UpdateCameraRotation(float rotationSpeedDegrees);
  void UpdateCameraPosition(float moveSpeed);
  void UpdateCameraVerticalMovement(float moveSpeed);

  void StartRenderTree();
  void RenderNode(AROctreeNode *node);
};
