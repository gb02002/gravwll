#pragma once
#include "raylib.h"
#include "utils/namespaces/MyMath.h"

struct MyCamera3D {
  Camera3D camera;
  MyMath::Vector3 position, target,
      up; // единичные вектора в цель взгляда и направления "вверха"
  float yaw = 0.0f,
        /* мера угла по оси Y */ pitch = 0.0f /* мера угла по оси X */;

  MyCamera3D(MyMath::Vector3 pos = MyMath::Vector3(0, 0, 0),
             MyMath::Vector3 tgt = MyMath::Vector3(0, 0, 1))
      : position(pos), target(tgt), up(MyMath::Vector3(0.0, 1.0, 0.0)) {
    camera.position = pos;
    camera.target = tgt;
    camera.up = up;
    camera.fovy = 45.0f;
    camera.projection = CAMERA_PERSPECTIVE;
  }

  operator Camera3D() const { return camera; }
};
