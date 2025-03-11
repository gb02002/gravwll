#include "cfx/cfx.h"
#include "gfx/gfx.h"
#include <chrono>
#include <raylib.h>
#include <thread>

void GfxEngine::Run() {
  using namespace std::chrono;
  milliseconds frameDuration(1000 / cfx.settings->FPS);
  auto nextFrameTime = high_resolution_clock::now() + frameDuration;

  while (cfx.state != EXIT) {
    Tick();

    std::this_thread::sleep_until(nextFrameTime);

    nextFrameTime += frameDuration;
  }
}
