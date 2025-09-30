#include "sorter.hpp"

#include <algorithm>
#include <thread>

namespace ae {

thread_local std::vector<container::element_type> sorter::present(ROBIN_HOOD_SPACE_MULT * ROBIN_HOOD_RANGE);

template <std::ranges::forward_range R>
static void merge(R&& left, R&& right) {
    auto it_left = std::ranges::begin(left);
    auto it_right = std::ranges::begin(right);
    auto end_left = std::ranges::end(left);
    auto end_right = std::ranges::end(right);

    while (it_left != end_left) {
        if (*it_left > *it_right) {
            auto val = *it_left;
            *it_left = *it_right;
            auto new_it_right = it_right;
            decltype(it_right) prev;
            do {
                prev = new_it_right;
                ++new_it_right;
                *prev = *new_it_right;
            } while (*new_it_right < val && new_it_right != end_right);
            *prev = val;
        }
        ++it_left;
    }
}

void sorter::sort(container& data, std::size_t num_threads) {
    std::vector<std::jthread> threads(num_threads);

    for (std::size_t thread_idx = 0; thread_idx < num_threads; thread_idx++) {
        threads[thread_idx] = std::jthread([&, thread_idx] {
            auto span = data.segment(thread_idx);
            do_radix<63>(span, 0, span.size() - 1);

            for (std::size_t i = 2; thread_idx % i == 0 && thread_idx + i / 2 < num_threads; i *= 2) {
                threads[thread_idx + i / 2].join();
                merge(data.to_view(thread_idx, thread_idx + i / 2), data.to_view(thread_idx + i / 2, thread_idx + i));
            }
        });
    }

    threads[0].join();
}

// Inspired by https://github.com/mlochbaum/rhsort
void sorter::do_robin_hood(std::span<container::element_type>& data, std::size_t start_index, std::size_t end_index) {
    std::size_t slots;
    if constexpr (ROBIN_HOOD_LIMIT_AUX_SPACE) {
        slots = (end_index - start_index + 1) * ROBIN_HOOD_SPACE_MULT;
    } else {
        slots = present.size();
    }
    auto min = data[start_index];
    auto max = min;

    for (std::size_t i = start_index + 1; i <= end_index; i++) {
        min = std::min(min, data[i]);
        max = std::max(max, data[i]);
    }

    // Widening range to always produce indices in [0, n) instead of [0, n].
    double range = max - min + 1;

    // To allow for differentiating between 0 as a value and an empty slot,
    // we set the most significant bit for all values in our robin hood array to 1.
    // We mask it out again when transferring the values back into the original array.
    // Note: Assuming at least one iteration of radix sort was executed,
    // we know that the msb will be consistent across the entire range.
    std::uint64_t msb_mask = (min & (1ull << 63)) ? ~0ull : ~(1ull << 63);

    for (std::size_t i = start_index; i <= end_index; i++) {
        auto new_val = data[i];
        auto idx = std::min<std::size_t>((new_val - min) / range * slots, slots - 1);
        std::uint64_t val = present[idx];
        new_val |= 1ull << 63;
        if (val) {
            if (val > new_val && idx > 0) {
                idx--;
                while (idx > 0 && (val = present[idx]) && val > new_val) {
                    idx--;
                }
            } else if (val < new_val && idx < slots - 1) {
                idx++;
                while (idx < slots - 1 && (val = present[idx]) && val < new_val) {
                    idx++;
                }
            }
        }
        val = present[idx];
        if (val) {
            // Either we encountered larger/equal values or we reached the end, either way we need to shift.

            // We need to either shift the larger values right (if they exist) or the smaller values left.
            // We shift into the direction with more space to avoid a situation where we cannot shift further.
            std::uint64_t next_val;
            auto shift_idx = idx;
            if (idx > slots / 2) {
                // Shift left.
                if (val > new_val) {
                    idx--;
                    val = present[idx];
                    shift_idx = idx;
                }
                do {
                    next_val = present[shift_idx - 1];
                    present[shift_idx - 1] = val;
                    val = next_val;
                    shift_idx--;
                } while (next_val);
            } else {
                // Shift right.
                if (val < new_val) {
                    idx++;
                    val = present[idx];
                    shift_idx = idx;
                }
                do {
                    next_val = present[shift_idx + 1];
                    present[shift_idx + 1] = val;
                    val = next_val;
                    shift_idx++;
                } while (next_val);
            }
        }

        present[idx] = new_val;
    }

    auto data_idx = start_index;
    for (std::size_t i = 0; i < slots; i++) {
        if (present[i]) {
            data[data_idx++] = present[i] & msb_mask;
            present[i] = 0;
        }
    }
}

}  // namespace ae
