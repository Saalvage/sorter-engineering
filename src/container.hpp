#pragma once

#include <cstddef>
#include <cstdint>
#include <iterator>
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

  // TODO You may also add additional functions (or data members).

 private:
  // TODO define your data layout
  // Your datastructure should consist of multiple blocks of data, which don't
  // necessarily have to be vectors.
  std::vector<std::vector<element_type>> placeholder_;

 public:
  [[nodiscard]] auto to_view() const {
    return std::views::join(placeholder_);
  }
};

}  // namespace ae
