#include "cfx/cfx.h"
#include "gfx/gfx.h"
#include <chrono>
#include <raylib.h>
#include <thread>

void GfxEngine::Run() {
  using namespace std::chrono;
  // Определяем длительность одного кадра в миллисекундах
  milliseconds frameDuration(1000 / cfx.FPS);
  // Вычисляем время для первого кадра
  auto nextFrameTime = high_resolution_clock::now() + frameDuration;

  while (cfx.state != EXIT) {
    // Обновление логики, отрисовка, и т.д.
    Tick();

    // Ждём до начала следующего кадра
    std::this_thread::sleep_until(nextFrameTime);

    // Обновляем время для следующего кадра
    nextFrameTime += frameDuration;
  }
}
