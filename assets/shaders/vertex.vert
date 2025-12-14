#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inMass;

layout(location = 0) out float outMass;

layout(set = 0, binding = 0) uniform SimpleUBO {
  mat4 mvp;
} ubo;

void main() {
  gl_Position = ubo.mvp * vec4(inPosition, 1.0);

  // Простой размер точки
  gl_PointSize = 10.0;

  outMass = inMass;
}
