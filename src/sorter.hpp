#pragma once

#include "container.hpp"

namespace ae {

class sorter {
public:
    void sort(container& data, int num_threads);

private:
    static constexpr int ROBIN_HOOD_RANGE = 1000000;
    static constexpr float ROBIN_HOOD_SPACE_MULT = 2.5f;

    static thread_local std::vector<container::element_type> presentr;

    template <typename T>
    T& lower_bits(std::uint64_t& ref) {
        constexpr std::size_t offset = 8 / sizeof(T) - 1;
        return *(reinterpret_cast<T*>(&ref) + offset);
    }

    // Inspired by https://en.wikipedia.org/wiki/Radix_sort
    template <int RADIX>
    void do_radix(container& data, std::size_t start_index, std::size_t end_index) {
        using internal_type = std::conditional_t<RADIX >= 32, std::uint64_t,
            std::conditional_t<RADIX >= 16, std::uint32_t,
                std::conditional_t<RADIX >= 8, std::uint16_t, std::uint8_t>>>;

        using internal_type_2 = std::conditional_t<RADIX >= 31, std::uint64_t,
            std::conditional_t<RADIX >= 15, std::uint32_t,
            std::conditional_t<RADIX >= 7, std::uint16_t, std::uint8_t>>>;

        if (ROBIN_HOOD_RANGE > 0 && end_index - start_index + 1 <= ROBIN_HOOD_RANGE && RADIX < 63) {
            do_robin_hood<internal_type_2>(data, start_index, end_index);
            return;
        }

        auto start_i = start_index;
        auto end_i = end_index;

        constexpr internal_type mask = 1ull << RADIX;
        while (start_i < end_i) {
            if (data[start_i] & mask) {
                std::swap(data[start_i], data[end_i--]);
            } else {
                start_i++;
            }
        }

        if constexpr (ROBIN_HOOD_RANGE <= 0 || RADIX != 0) {
          auto new_end = lower_bits<internal_type>(data[start_i]) & mask ? start_i - 1 : start_i;
            if (start_i != 0 && start_index < new_end) {
                do_radix<RADIX - 1>(data, start_index, new_end);
            }
            if (new_end + 1 < end_index) {
                do_radix<RADIX - 1>(data, new_end + 1, end_index);
            }
        }
    }

    // Inspired by https://github.com/mlochbaum/rhsort
    template <typename T>
    void do_robin_hood(container& data, std::size_t start_index, std::size_t end_index) {
		T* present = reinterpret_cast<T*>(presentr.data());
        std::size_t slots =
#if AE_ROBIN_HOOD_LIMIT_AUX_SPACE
        (end_index - start_index + 1) * ROBIN_HOOD_SPACE_MULT;
#else
            presentr.size() * 8 / sizeof(T);
#endif
        auto min = lower_bits<T>(data[start_index]);
        auto max = min;

        for (std::size_t i = start_index + 1; i <= end_index; i++) {
            min = std::min(min, lower_bits<T>(data[i]));
            max = std::max(max, lower_bits<T>(data[i]));
        }

        // Widening range to always produce indices in [0, n) instead of [0, n].
        double range = max - min + 1;

        // To allow for differentiating between 0 as a value and an empty slot,
        // we set the most significant bit for all values in our robin hood array to 1.
        // We mask it out again when transferring the values back into the original array.
        // Note: Assuming at least one iteration of radix sort was executed,
        // we know that the msb will be consistent across the entire range.
        constexpr std::size_t shifted = 1ull << (sizeof(T) * 8 - 1);
        T msb_mask = (min & shifted) ? ~0ull : ~shifted;

        for (std::size_t i = start_index; i <= end_index; i++) {
            auto new_val = lower_bits<T>(data[i]);
            auto idx = std::min<std::size_t>((new_val - min) / range * slots, slots - 1);
            T val = present[idx];
            new_val |= shifted;
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
                lower_bits<T>(data[data_idx++]) = present[i] & msb_mask;
                present[i] = 0;
            }
        }
    }
};

}  // namespace ae
