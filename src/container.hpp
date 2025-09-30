#pragma once

#include <cstdint>
#include <ranges>
#include <span>
#include <vector>

namespace ae {

class sorter;

class container {
  friend class sorter;

 public:
  using element_type = std::uint64_t;

  explicit container(std::span<const element_type> data);

  element_type& operator[](std::size_t idx);

  std::size_t size() const;

 private:
  std::size_t elements_per_block;
  std::vector<std::vector<element_type>> blocks;
  std::size_t total_size;

 public:
  [[nodiscard]] auto to_view() const {
    return std::views::join(blocks);
  }
};

}  // namespace ae
