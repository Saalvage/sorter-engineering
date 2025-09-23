#include "sorter.hpp"

#include <algorithm>
#include <iterator>

namespace ae {

void sorter::sort(container& data) {
    do_radix(data, 63, 0, data.size() - 1);
}

void sorter::do_radix(container& data, int radix, std::size_t start_index, std::size_t end_index) {
    auto start_i = start_index;
    auto end_i = end_index;

    auto mask = 1ull << radix;
    while (start_i < end_i) {
        if (data[start_i] & mask) {
            std::swap(data[start_i], data[end_i--]);
        } else {
            start_i++;
        }
    }

    if (radix != 0) {
        auto new_end = data[start_i] & mask ? start_i - 1 : start_i;
        if (start_i != 0 && start_index < new_end) {
            do_radix(data, radix - 1, start_index, new_end);
        }
        if (new_end + 1 < end_index) {
            do_radix(data, radix - 1, new_end + 1, end_index);
        }
    }
}


}  // namespace ae
