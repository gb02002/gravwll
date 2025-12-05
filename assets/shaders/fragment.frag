#version 460
#extension GL_EXT_buffer_reference : require

layout(location = 0) in float inMass;

layout(location = 0) out vec4 outColor;

void main() {
  float c = clamp(inMass * 0.001, 0.0, 1.0);
  outColor = vec4(c, 0.2, 1.0 - c, 1.0);
}
