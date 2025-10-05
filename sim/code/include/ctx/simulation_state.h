#pragma once

#include <atomic>
enum class STATE { RUN, STOP, EXIT, ERROR, INITIALIZING, WARM_UP };

// Общее состояние симуляции, разделяемое между компонентами
struct SimulationState {
  STATE current_state = STATE::INITIALIZING;
  std::atomic<bool> should_exit{false}; // для межпоточного взаимодействия

  // Методы для безопасного изменения состояния
  void set_state(STATE new_state) { current_state = new_state; }
  bool is_running() const { return current_state == STATE::RUN; }
  void request_exit() { should_exit = true; }
};
