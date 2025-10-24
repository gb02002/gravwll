#version 450

layout(location = 0) in float inMass;
layout(location = 0) out vec4 outColor;

void main() {
  // масса = насыщенность белого
  float intensity = clamp(inMass, 0.0, 1.0);
  outColor = vec4(vec3(intensity), 1.0);
}
