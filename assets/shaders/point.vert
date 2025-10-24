#version 450

layout(location = 0) in dvec3 inPosition; // твои координаты
layout(location = 1) in float inMass; // масса
layout(location = 0) out float outMass;

layout(set = 0, binding = 0) uniform UBO {
  mat4 mvp;
} ubo;

void main() {
  gl_Position = ubo.mvp * vec4(inPosition, 1.0);
  outMass = inMass;
}
