#include "sorter.hpp"

#include <algorithm>

#define AE_ROBIN_HOOD_LIMIT_AUX_SPACE false

namespace ae {

thread_local std::vector<container::element_type> sorter::presentr(ROBIN_HOOD_SPACE_MULT * ROBIN_HOOD_RANGE);

void sorter::sort(container& data, int num_threads) {
    do_radix<63>(data, 0, data.size() - 1);
}

}  // namespace ae
