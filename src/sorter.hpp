#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <stack>

#include "container.hpp"

namespace ae {

class sorter {
 public:
  void sort(container& data, std::size_t num_threads);

private:
  static constexpr bool ROBIN_HOOD_LIMIT_AUX_SPACE = true;
  static constexpr int ROBIN_HOOD_RANGE = 1'000'000;
  static constexpr float ROBIN_HOOD_SPACE_MULT = 2.f;

  struct job {
    int radix;
    std::size_t start_index;
    std::size_t end_index;
  };

  static thread_local std::vector<container::element_type> robin_hood_auxiliary;

  std::stack<job, std::vector<job>> jobs;
  std::mutex jobs_mutex;
  std::condition_variable jobs_cv;
  std::atomic_bool is_sorting;
  std::atomic_uint waiting_count;

  void do_radix(container& data, int radix, std::size_t start_index, std::size_t end_index);
  void do_robin_hood(container& data, std::size_t start_index, std::size_t end_index);
};

}  // namespace ae
