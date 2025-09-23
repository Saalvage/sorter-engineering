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

  element_type& operator[](std::size_t idx);

  std::size_t size() const;

 private:
  // TODO define your data layout
  // Your datastructure should consist of multiple blocks of data, which don't
  // necessarily have to be vectors.
  std::vector<element_type> placeholder_;
  std::size_t size_;

 public:
  [[nodiscard]] auto to_view() const {
    return std::views::all(placeholder_);
  }
};

}  // namespace ae
