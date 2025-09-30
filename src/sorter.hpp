#pragma once

#include <mutex>
#include <condition_variable>
#include <atomic>
#include <stack>

#include "container.hpp"

namespace ae {

class sorter {
 public:
  void sort(container& data, int num_threads);

private:
  struct job {
	int radix;
	std::size_t start_index;
	std::size_t end_index;
  };

  static thread_local std::vector<container::element_type> present;

  std::stack<job, std::vector<job>> jobs;
  std::mutex jobs_mutex;
  std::condition_variable jobs_cv;
  std::atomic_bool is_sorting;
  std::atomic_int waiting_count;

  void do_radix(container& data, int radix, std::size_t start_index, std::size_t end_index);
  void do_robin_hood(container& data, std::size_t start_index, std::size_t end_index);
};

}  // namespace ae
