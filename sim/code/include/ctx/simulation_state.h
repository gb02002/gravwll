#pragma once

#include <atomic>
#include <iostream>
enum class STATE { RUN, STOP, EXIT, ERROR, INITIALIZING, WARM_UP };

// Общее состояние симуляции, разделяемое между компонентами
struct SimulationState {
  STATE current_state = STATE::INITIALIZING;
  std::atomic<bool> should_exit{false};

  void set_state(STATE new_state) { current_state = new_state; }
  bool is_running() const { return current_state == STATE::RUN; }
  void request_exit() {
    std::cout << "request_exit invoked" << std::endl;
    should_exit = true;
    current_state = STATE::EXIT;
  }
};
