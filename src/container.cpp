#include "container.hpp"

#include <cstddef>
#include <span>

namespace ae {

container::container(std::span<const element_type> data) {
  // TODO create your datastructure from the given data

  // The code below is a simple example splitting the data into 16 blocks,
  // but you may find other options better suited for your sorting algorithm.
  constexpr std::size_t num_blocks = 16;
  const std::ptrdiff_t elements_per_block = (data.size() + num_blocks - 1) / num_blocks;

  for (auto first = data.begin(); first < data.end();) {
    const auto last = (data.end() - first) < elements_per_block ? data.end() : first + elements_per_block;
    placeholder_.emplace_back(first, last);
    first = last;
  }
}

}  // namespace ae
