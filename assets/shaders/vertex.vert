#version 460

layout(location = 0) in vec3 inPosition;
layout(location = 1) in float inMass;
layout(location = 2) in uint inVisualIdLow; // uint for low
layout(location = 3) in uint inVisualIdHigh; // uint for high

layout(location = 0) out float outMass;
layout(location = 1) out float outPointSize;
layout(location = 2) flat out uvec2 outVisualId;

layout(set = 0, binding = 0) uniform CameraUBO {
  mat4 view;
  mat4 projection;
  vec4 camera_pos;
  float point_size;
  float time;
  float zoom_level;
  float brightness;
} camera;

uvec2 pack_visual_id(uint low, uint high) {
  return uvec2(low, high);
}

// Функции для извлечения полей из VisualID
uint extract_cat(uvec2 visual_id) {
  return (visual_id.x >> 0) & 0xF; // Младшие 4 бита
}

uint extract_subtype(uvec2 visual_id) {
  return (visual_id.x >> 4) & 0xF; // Биты 4-7
}

uint extract_shader(uvec2 visual_id) {
  return (visual_id.x >> 8) & 0xFF; // Биты 8-15
}

// ... остальные функции для извлечения полей ...

float compute_point_size(float mass, float distance, uint cat, uint subtype) {
  float base_size = camera.point_size;

  // Разные базовые размеры для разных категорий
  float category_size = 1.0;
  if (cat == 0) { // Звезды
    category_size = 2.0;
  } else if (cat == 1) { // Планеты
    category_size = 1.5;
  } else if (cat == 2) { // Астероиды/планетезимали
    category_size = 0.5;
  }

  float mass_factor = log(mass + 1.0) * 0.5;
  float distance_attenuation = 10.0 / (distance + 1.0);
  float zoom_factor = 1.0 / max(camera.zoom_level, 0.1);

  return clamp(base_size * category_size * mass_factor *
      distance_attenuation * zoom_factor, 1.0, 64.0);
}

void main() {
  vec3 world_pos = inPosition;
  vec4 view_pos = camera.view * vec4(world_pos, 1.0);
  gl_Position = camera.projection * view_pos;

  float distance = length(camera.camera_pos.xyz - world_pos);

  // Извлекаем категорию и подтип
  uvec2 visual_id = uvec2(inVisualIdLow, inVisualIdHigh);
  uint cat = extract_cat(visual_id);
  uint subtype = extract_subtype(visual_id);

  // Размер с учетом категории
  outPointSize = compute_point_size(inMass, distance, cat, subtype);
  gl_PointSize = outPointSize;

  // Передаем данные дальше
  outMass = inMass;
  outVisualId = visual_id;

  // Пульсация только для звезд (cat == 0)
  if (cat == 0) {
    float pulse_freq = 2.0;
    if (subtype == 0) pulse_freq = 1.5; // Красные карлики
    if (subtype == 2) pulse_freq = 3.0; // Голубые гиганты

    float pulse = sin(camera.time * pulse_freq + inMass * 10.0) * 0.1 + 0.9;
    gl_PointSize *= pulse;
  }
}
