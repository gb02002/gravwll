#version 460

layout(location = 0) in float inMass;
layout(location = 1) in float inPointSize;
layout(location = 2) flat in uvec2 inVisualId;

layout(location = 0) out vec4 outColor;

// Категории (должны совпадать с C++ кодом)
const uint CAT_STAR = 0;
const uint CAT_PLANET = 1;
const uint CAT_ASTEROID = 2;
const uint CAT_DARK_MATTER = 3;
const uint CAT_BLACK_HOLE = 4;
const uint CAT_NEBULA = 5;
const uint CAT_DEBRIS = 6;
const uint CAT_UNKNOWN = 15;

// Подтипы звезд
const uint SUBTYPE_STAR_RED_DWARF = 0;
const uint SUBTYPE_STAR_YELLOW = 1;
const uint SUBTYPE_STAR_BLUE_GIANT = 2;
const uint SUBTYPE_STAR_RED_GIANT = 3;
const uint SUBTYPE_STAR_WHITE_DWARF = 4;
const uint SUBTYPE_STAR_NEUTRON = 5;
const uint SUBTYPE_STAR_BLACK_HOLE = 6;

// Подтипы планет
const uint SUBTYPE_PLANET_TERRESTRIAL = 0;
const uint SUBTYPE_PLANET_GAS_GIANT = 1;
const uint SUBTYPE_PLANET_ICE_GIANT = 2;
const uint SUBTYPE_PLANET_DWARF = 3;
const uint SUBTYPE_PLANET_OCEAN = 4;
const uint SUBTYPE_PLANET_DESERT = 5;

// Подтипы астероидов
const uint SUBTYPE_ASTEROID_ROCKY = 0;
const uint SUBTYPE_ASTEROID_METALLIC = 1;
const uint SUBTYPE_ASTEROID_ICY = 2;
const uint SUBTYPE_ASTEROID_CARBONACEOUS = 3;

// Функции извлечения (те же что в вершинном шейдере)
uint extract_cat(uvec2 visual_id) {
  return (visual_id.x >> 0) & 0xF;
}

uint extract_subtype(uvec2 visual_id) {
  return (visual_id.x >> 4) & 0xF;
}

uint extract_shader(uvec2 visual_id) {
  return (visual_id.x >> 8) & 0xFF;
}

// ... остальные функции извлечения ...

// Цветовые палитры для каждой категории
vec3 get_star_color(uint subtype, float mass) {
  if (subtype == SUBTYPE_STAR_RED_DWARF) {
    return vec3(1.0, 0.3, 0.2); // Красный
  } else if (subtype == SUBTYPE_STAR_YELLOW) {
    return vec3(1.0, 0.9, 0.5); // Желто-белый
  } else if (subtype == SUBTYPE_STAR_BLUE_GIANT) {
    return vec3(0.6, 0.8, 1.0); // Голубой
  } else if (subtype == SUBTYPE_STAR_RED_GIANT) {
    return vec3(1.0, 0.5, 0.3); // Оранжево-красный
  } else if (subtype == SUBTYPE_STAR_WHITE_DWARF) {
    return vec3(1.0, 1.0, 1.0); // Белый
  } else if (subtype == SUBTYPE_STAR_NEUTRON) {
    return vec3(0.8, 0.9, 1.0); // Голубовато-белый
  } else {
    return vec3(1.0, 0.8, 0.6); // По умолчанию
  }
}

vec3 get_planet_color(uint subtype, float mass) {
  if (subtype == SUBTYPE_PLANET_TERRESTRIAL) {
    return vec3(0.3, 0.7, 0.4); // Зеленый (землеподобный)
  } else if (subtype == SUBTYPE_PLANET_GAS_GIANT) {
    return vec3(0.8, 0.7, 0.5); // Коричнево-желтый (Юпитер)
  } else if (subtype == SUBTYPE_PLANET_ICE_GIANT) {
    return vec3(0.5, 0.7, 1.0); // Голубой (Нептун)
  } else if (subtype == SUBTYPE_PLANET_DWARF) {
    return vec3(0.6, 0.6, 0.6); // Серый (Плутон)
  } else if (subtype == SUBTYPE_PLANET_OCEAN) {
    return vec3(0.2, 0.4, 0.9); // Синий (океанический)
  } else if (subtype == SUBTYPE_PLANET_DESERT) {
    return vec3(0.9, 0.7, 0.4); // Песочный
  } else {
    return vec3(0.5, 0.5, 0.5); // Серый по умолчанию
  }
}

vec3 get_asteroid_color(uint subtype, float mass) {
  if (subtype == SUBTYPE_ASTEROID_ROCKY) {
    return vec3(0.5, 0.4, 0.3); // Коричневый
  } else if (subtype == SUBTYPE_ASTEROID_METALLIC) {
    return vec3(0.7, 0.7, 0.7); // Металлический серый
  } else if (subtype == SUBTYPE_ASTEROID_ICY) {
    return vec3(0.8, 0.9, 1.0); // Ледяной голубой
  } else if (subtype == SUBTYPE_ASTEROID_CARBONACEOUS) {
    return vec3(0.2, 0.2, 0.2); // Темный углеродистый
  } else {
    return vec3(0.6, 0.6, 0.6); // Серый по умолчанию
  }
}

// Главная функция выбора цвета
vec3 get_base_color(uvec2 visual_id, float mass) {
  uint cat = extract_cat(visual_id);
  uint subtype = extract_subtype(visual_id);

  if (cat == CAT_STAR) {
    return get_star_color(subtype, mass);
  } else if (cat == CAT_PLANET) {
    return get_planet_color(subtype, mass);
  } else if (cat == CAT_ASTEROID) {
    return get_asteroid_color(subtype, mass);
  } else if (cat == CAT_DARK_MATTER) {
    return vec3(0.3, 0.1, 0.5); // Фиолетовый
  } else if (cat == CAT_BLACK_HOLE) {
    return vec3(0.0, 0.0, 0.0); // Черный
  } else if (cat == CAT_NEBULA) {
    return vec3(0.8, 0.4, 1.0); // Фиолетово-розовый
  } else if (cat == CAT_DEBRIS) {
    return vec3(0.4, 0.3, 0.2); // Коричневый
  } else {
    return vec3(1.0, 1.0, 1.0); // Белый по умолчанию
  }
}

// Расширенная функция свечения с учетом категории
float get_glow_intensity(uvec2 visual_id, float dist) {
  uint cat = extract_cat(visual_id);

  float base_glow = exp(-dist * dist / 0.1);

  if (cat == CAT_STAR) {
    return base_glow * 2.0; // Звезды светят ярче
  } else if (cat == CAT_BLACK_HOLE) {
    return base_glow * 1.5; // Аккреционный диск
  } else if (cat == CAT_NEBULA) {
    return base_glow * 0.8; // Туманности более рассеянные
  } else {
    return base_glow * 0.5; // Обычные объекты
  }
}

// Мерцание для разных типов объектов
float get_twinkle(uvec2 visual_id, float time, vec2 coord) {
  uint cat = extract_cat(visual_id);
  uint subtype = extract_subtype(visual_id);

  if (cat == CAT_STAR) {
    // Звезды мерцают
    float frequency = 3.0 + float(subtype) * 0.5;
    float base = sin(time * frequency) * 0.3 + 0.7;

    // Быстрое мерцание для некоторых типов
    if (subtype == SUBTYPE_STAR_NEUTRON) {
      float fast = sin(time * 10.0) * 0.1 + 0.9;
      base *= fast;
    }
    return base;
  } else if (cat == CAT_BLACK_HOLE) {
    // Черные дыры могут иметь переменную яркость аккреционного диска
    return sin(time * 0.5) * 0.2 + 0.8;
  }

  return 1.0; // Остальные не мерцают
}

void main() {
  // Координаты и расстояние от центра
  vec2 coord = gl_PointCoord - vec2(0.5);
  float dist = length(coord);

  // Отбрасываем углы для круглых точек
  if (dist > 0.5) {
    discard;
  }

  // Базовый цвет на основе visual_id
  vec3 base_color = get_base_color(inVisualId, inMass);

  // Яркость центра
  float center_brightness = 1.0 - smoothstep(0.0, 0.7, dist);

  // Свечение с учетом категории
  float glow_intensity = get_glow_intensity(inVisualId, dist);

  // Мерцание
  float twinkle_factor = get_twinkle(inVisualId, 0.0, coord); // camera.time

  // Финальная яркость
  float brightness = center_brightness * glow_intensity * twinkle_factor;

  // Коррекция для размера
  float size_correction = clamp(10.0 / inPointSize, 0.7, 1.5);

  // Финальный цвет
  vec3 final_color = base_color * brightness * size_correction;

  // Гарантируем минимальную яркость
  final_color = max(final_color, vec3(0.1));

  // Альфа (прозрачность краев)
  float alpha = 1.0 - smoothstep(0.4, 0.5, dist);

  // Маленькие точки полностью непрозрачны
  if (inPointSize < 2.0) {
    alpha = 1.0;
  }

  outColor = vec4(final_color, alpha);
}
