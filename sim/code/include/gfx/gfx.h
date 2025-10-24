#pragma once
#include "ctx/ctx.h"
#include "ctx/simulation_state.h"
#include "ds/tree/octree.h"
#include "gfx/camera.h"

class GfxEngine {
public:
  explicit GfxEngine(GfxCtx &g_ctx, SimulationState &s_state, AROctree &tree);

  void init();
  void tick();
  void run();
  void clean_up();

private:
  GfxCtx &g_ctx;            // gfx context
  SimulationState &s_state; // shared state
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
  void init_vulkan1();
  void init_vulkan();
};
