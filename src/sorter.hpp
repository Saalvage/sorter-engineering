#pragma once

#include "container.hpp"

namespace ae {

class sorter {
 public:
  void sort(container& data, int num_threads);

private:
  static thread_local std::vector<container::element_type> present;
  void do_radix(container& data, int radix, std::size_t start_index, std::size_t end_index);
  void do_robin_hood(container& data, std::size_t start_index, std::size_t end_index);
};

}  // namespace ae
