#include "container.hpp"

#include <span>

namespace ae {

container::container(std::span<const element_type> data, std::size_t num_threads) {
    std::size_t num_blocks = num_threads;
    const std::ptrdiff_t elements_per_block = (data.size() + num_blocks - 1) / num_blocks;

    for (auto first = data.begin(); first < data.end();) {
        const auto last = (data.end() - first) < elements_per_block ? data.end() : first + elements_per_block;
        segments.emplace_back(first, last);
        first = last;
    }
}

std::span<container::element_type> container::segment(int idx) {
    return segments[idx];
}


}  // namespace ae
