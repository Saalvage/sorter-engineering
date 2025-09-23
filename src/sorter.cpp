#include "sorter.hpp"

#include <algorithm>
#include <iterator>

namespace ae {

void sorter::sort(container& data) {
  // TODO Implement your sorting algorithm
  for (auto i = 1uz; i < data.placeholder_.size(); ++i) {
    std::ranges::copy(data.placeholder_[i], std::back_inserter(data.placeholder_[0]));
    data.placeholder_[i].clear();
  }
  std::ranges::sort(data.placeholder_[0]);
}

}  // namespace ae
