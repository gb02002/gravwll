#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inMass;

layout(location = 0) out float outMass;

layout(set = 0, binding = 0) uniform CameraUBO {
  mat4 view;
  mat4 projection;
  vec4 camera_pos;
  float point_size;
  float time;
} camera;

void main() {
  // Простое преобразование: view * projection
  gl_Position = camera.projection * camera.view * vec4(inPosition, 1.0);

  // Фиксированный размер точки
  gl_PointSize = camera.point_size;

  outMass = inMass;
}
