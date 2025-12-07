#version 460

layout(location = 1) in float inMass;
layout(location = 0) out vec4 outColor;

void main() {
  // Яркие, сильно различающиеся цвета
  if (inMass < 1.5) {
    outColor = vec4(1.0, 0.0, 0.0, 1.0); // Красный
  } else if (inMass < 2.5) {
    outColor = vec4(0.0, 1.0, 0.0, 1.0); // Зеленый
  } else if (inMass < 3.5) {
    outColor = vec4(0.0, 0.0, 1.0, 1.0); // Синий
  } else if (inMass < 4.5) {
    outColor = vec4(1.0, 1.0, 0.0, 1.0); // Желтый
  } else {
    outColor = vec4(1.0, 0.0, 1.0, 1.0); // Пурпурный (центр)
  }
}
