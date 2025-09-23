#include "container.hpp"

#include <cstddef>
#include <span>

namespace ae {

// The code below is a simple example splitting the data into 16 blocks,
// but you may find other options better suited for your sorting algorithm.
static constexpr std::size_t num_blocks = 16;

container::container(std::span<const element_type> data) {
  // TODO create your datastructure from the given data
  const std::ptrdiff_t elements_per_block = (data.size() + num_blocks - 1) / num_blocks;

  for (auto first = data.begin(); first < data.end(); ++first) {
      placeholder_.emplace_back(*first);
  }

  size_ = data.size();
}

std::size_t container::size() const {
    return size_;
}

container::element_type& container::operator[](std::size_t idx) {
	return placeholder_[idx];
}


}  // namespace ae
