#include "gfx/gfx.h"
#include <chrono>
#include <raylib.h>
#include <thread>

void GfxEngine::Run() {
  using namespace std::chrono;
  auto start_reander_time = high_resolution_clock::now();
  auto stepping = milliseconds(1000 / g_ctx.desired_fps);
  auto next_frame_time = start_reander_time + stepping;

  while (s_state.is_running()) {
    Tick();
    std::this_thread::sleep_until(next_frame_time);
    next_frame_time += stepping;
  }
}
