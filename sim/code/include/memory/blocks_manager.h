#pragma once
#include "ds/storage/particleBlock.h"
#include "memory/blocks_arena.h"
#include <cstddef>
#include <limits>
#include <vector>

class BlockMemoryManager {
public:
  explicit BlockMemoryManager(size_t N_body);
  ~BlockMemoryManager() = default;

  ParticleBlock *create_block(MortonKey key);
  void destroy_block(ParticleBlock *p_bl);

  ParticleBlock *get_block_data(size_t inx);
  const ParticleBlock *get_block_data(size_t inx) const;
  MortonKey get_block_key(size_t inx) const;

  void swap_blocks(size_t inx_a, size_t inx_b);
  void compact();

  class Iterator;
  Iterator begin();
  Iterator end();

  size_t get_capacity() const { return arena_.capacity; };
  size_t get_used_blocks() const {
    return arena_.current_counter;
    ;
  }

private:
  static size_t compute_capacity(size_t N_body) {
    // Базовое количество блоков (округление вверх)
    const size_t base_blocks = (N_body + 15) / 16;

    // Эмпирические коэффициенты на основе тестирования октодеревьев:
    // - Множитель 4-6 хорошо работает для средних деревьев
    // - Учитываем что при делении получается 8 новых блоков
    // - Минимальный размер дает место для начального роста

    const size_t multiplier = 6;  // эмпирически подобранный множитель
    const size_t min_blocks = 32; // минимум для 2 уровней дерева (1 + 8)
    const size_t max_blocks = std::max(
        N_body * 2, std::numeric_limits<size_t>::max()); // разумный предел

    size_t estimated_blocks = base_blocks * multiplier;

    // Гарантируем, что выделенного места хватит хотя бы на 2 уровня деления
    estimated_blocks = std::max(estimated_blocks, min_blocks);

    // Защита от астрономических значений
    return std::min(estimated_blocks, max_blocks);
  }
  BlocksAllocator arena_;
  std::vector<bool> active_blocks_;
  std::vector<MortonKey> block_keys_;
};

class BlockMemoryManager::Iterator {
public:
  Iterator(BlockMemoryManager *manager, size_t index)
      : manager_(manager), current_index_(index) {}

  ParticleBlock &operator*() {
    return *manager_->get_block_data(current_index_);
  }
  ParticleBlock *operator->() {
    return manager_->get_block_data(current_index_);
  }
  Iterator &operator++() {
    do {
      current_index_++;
    } while (current_index_ < manager_->get_capacity() &&
             !manager_->active_blocks_[current_index_]);
    return *this;
  }
  bool operator!=(const Iterator &other) const {
    return current_index_ != other.current_index_;
  }

private:
  BlockMemoryManager *manager_;
  size_t current_index_;
};
