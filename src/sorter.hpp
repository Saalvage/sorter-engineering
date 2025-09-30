#pragma once

#include "container.hpp"

namespace ae {

class sorter {
public:
    void sort(container& data, std::size_t num_threads);

private:
    static thread_local std::vector<container::element_type> present;

    static constexpr int ROBIN_HOOD_RANGE = 1000000;
    static constexpr float ROBIN_HOOD_SPACE_MULT = 2.f;
    static constexpr bool ROBIN_HOOD_LIMIT_AUX_SPACE = true;

    // Inspired by https://en.wikipedia.org/wiki/Radix_sort
    template <int RADIX>
    void do_radix(std::span<container::element_type>& data, std::size_t start_index, std::size_t end_index) {
        if (ROBIN_HOOD_RANGE > 0 && end_index - start_index + 1 <= ROBIN_HOOD_RANGE && RADIX < 63) {
            do_robin_hood(data, start_index, end_index);
            return;
        }

        auto start_i = start_index;
        auto end_i = end_index;

        constexpr auto mask = 1ull << RADIX;
        while (start_i < end_i) {
            if (data[start_i] & mask) {
                std::swap(data[start_i], data[end_i--]);
            } else {
                start_i++;
            }
        }

        if constexpr (RADIX != 0) {
            auto new_end = data[start_i] & mask ? start_i - 1 : start_i;
            if (start_i != 0 && start_index < new_end) {
                do_radix<RADIX - 1>(data, start_index, new_end);
            }
            if (new_end + 1 < end_index) {
                do_radix<RADIX - 1>(data, new_end + 1, end_index);
            }
        }
    }

    void do_robin_hood(std::span<container::element_type>& data, std::size_t start_index, std::size_t end_index);
};

}  // namespace ae
