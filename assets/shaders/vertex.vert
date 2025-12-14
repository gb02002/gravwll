#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inMass;

layout(location = 0) out float outMass;
layout(location = 1) out float outPointSize;

// layout(set = 0, binding = 0) uniform CameraUBO {
//   mat4 view;
//   mat4 projection;
//   vec4 camera_pos; // Объявлено здесь как camera_pos
//   float time;
// } camera; // Но экземпляр называется "camera"

layout(set = 0, binding = 0) uniform UBO {
  mat4 mvp;
} ubo;

void main1() {
  gl_Position = ubo.mvp * vec4(inPosition, 1.0);
  outMass = inMass;
}

void main() {
  vec3 world_pos = inPosition;

  vec4 view_pos = camera.view * vec4(world_pos, 1.0);
  gl_Position = camera.projection * view_pos;

  // Используем camera.camera_pos, а не просто camera_pos
  float distance = length(camera.camera_pos.xyz - world_pos);
  float size_attenuation = 100.0 / (distance + 1.0);

  float mass_factor = sqrt(inMass) * 0.5;
  gl_PointSize = size_attenuation * mass_factor;

  outMass = inMass;
  outPointSize = gl_PointSize;

  float pulse = sin(camera.time * 2.0 + inMass) * 0.1 + 0.9;
  gl_PointSize *= pulse;
}
