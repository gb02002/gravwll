#include "gfx/camera.h"

Camera3D ConvertCamera(const MyCamera3D &myCam) {
  Camera3D libCam;
  // Используем оператор преобразования для векторов, если он определён
  libCam.position = myCam.position;
  libCam.target = myCam.target;
  libCam.up = myCam.up;
  libCam.fovy = 45.0f; // или другое значение из настроек
  libCam.projection = CAMERA_PERSPECTIVE;
  return libCam;
}
