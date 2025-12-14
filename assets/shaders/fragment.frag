#version 460

layout(location = 0) in float inMass;
layout(location = 1) in float inPointSize;

layout(location = 0) out vec4 outColor;

vec3 cool = vec3(0.0, 0.2, 1.0);
vec3 hot = vec3(1.0, 0.0, 0.0);
vec3 warm = vec3(1.0, 0.8, 0.2);

void main() {
  vec2 coord = gl_PointCoord - vec2(0.5);
  float dist = length(coord);

  if (dist > 0.5) {
    discard;
  }

  float t = clamp(inMass * 0.001, 0.0, 1.0);

  vec3 color_mass = mix(cool, hot, t);

  vec3 color_gradient;
  if (t < 0.5) {
    color_gradient = mix(cool, warm, t * 2.0);
  } else {
    color_gradient = mix(warm, hot, (t - 0.5) * 2.0);
  }

  float edge_glow = 1.0 - smoothstep(0.3, 0.5, dist);
  color_gradient += edge_glow * 0.3;

  float size_factor = clamp(inPointSize / 10.0, 0.5, 2.0);

  outColor = vec4(color_gradient * size_factor, 1.0);
}
