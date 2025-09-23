#pragma once

#include "container.hpp"

namespace ae {

class sorter {
 public:
  void sort(container& data);
  void do_radix(container& data, int radix, std::size_t start_index, std::size_t end_index);
};

}  // namespace ae
