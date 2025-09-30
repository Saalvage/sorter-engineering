#include "container.hpp"

#include <span>

namespace ae {

static constexpr std::size_t NUM_BLOCKS = 1;

container::container(std::span<const element_type> data)
	: elements_per_block((data.size() + NUM_BLOCKS - 1) / NUM_BLOCKS), total_size(data.size()) {
    for (auto first = data.begin(); first < data.end();) {
        const auto last = (data.end() - first) < elements_per_block ? data.end() : first + elements_per_block;
        blocks.emplace_back(first, last);
        first = last;
    }
}

std::size_t container::size() const {
    return total_size;
}

container::element_type& container::operator[](std::size_t idx) {
    // We don't utilize the partitioning into blocks, so we just abstract away from it.
    if constexpr (NUM_BLOCKS == 1) {
        return blocks[0][idx];
	} else {
        return blocks[idx / elements_per_block][idx % elements_per_block];
	}
}

}  // namespace ae
