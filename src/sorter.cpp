#include "sorter.hpp"

#include <algorithm>
#include <thread>

namespace ae {

thread_local std::vector<container::element_type> sorter::robin_hood_auxiliary(ROBIN_HOOD_SPACE_MULT * ROBIN_HOOD_RANGE);

void sorter::sort(container& data, std::size_t num_threads) {
    is_sorting = true;
    waiting_count = 0;

    std::vector<std::jthread> threads(num_threads);
    for (std::size_t i = 0; i < num_threads; i++) {
        threads[i] = std::jthread([&, i] {
            if (i == 0) {
                do_radix(data, 63, 0, data.size() - 1);
            }

            while (true) {
                std::unique_lock lock{ jobs_mutex };
                if (jobs.empty()) {
                    if (++waiting_count == num_threads) {
                        is_sorting = false;
                        jobs_cv.notify_all();
                        break;
                    }
                    jobs_cv.wait(lock, [&] { return !is_sorting || !jobs.empty(); });
                    --waiting_count;
                }

                if (!is_sorting) {
                    break;
                }

                auto my_job = jobs.top();
                jobs.pop();
                lock.unlock();
                do_radix(data, my_job.radix, my_job.start_index, my_job.end_index);
            }
        });
    }

    for (std::size_t i = 0; i < num_threads; i++) {
        threads[i].join();
    }
}

// Inspired by https://en.wikipedia.org/wiki/Radix_sort
void sorter::do_radix(container& data, int radix, std::size_t start_index, std::size_t end_index) {
    if (ROBIN_HOOD_RANGE > 0 && end_index - start_index + 1 <= ROBIN_HOOD_RANGE && radix < 63) {
        do_robin_hood(data, start_index, end_index);
        return;
    }

    auto start_i = start_index;
    auto end_i = end_index;

    auto mask = 1ull << radix;
    while (start_i < end_i) {
        if (data[start_i] & mask) {
            std::swap(data[start_i], data[end_i]);
            end_i--;
        } else {
            start_i++;
        }
    }

    if (ROBIN_HOOD_RANGE <= 0 || radix != 0) {
        radix--;
        auto new_end = data[start_i] & mask ? start_i - 1 : start_i;
        bool has_cont = start_i != 0 && start_index < new_end;
        if (new_end + 1 < end_index) {
            if (has_cont) {
                {
                    std::lock_guard lock{ jobs_mutex };
                    jobs.emplace(radix, new_end + 1, end_index);
                }
                jobs_cv.notify_one();
                do_radix(data, radix, start_index, new_end);
            } else {
                do_radix(data, radix, new_end + 1, end_index);
            }
        } else if (has_cont) {
            do_radix(data, radix, start_index, new_end);
        }
    }
}

// Inspired by https://github.com/mlochbaum/rhsort
void sorter::do_robin_hood(container& data, std::size_t start_index, std::size_t end_index) {
    std::size_t slots;
    if constexpr (ROBIN_HOOD_LIMIT_AUX_SPACE) {
        slots = (end_index - start_index + 1) * ROBIN_HOOD_SPACE_MULT;
    } else {
        slots = robin_hood_auxiliary.size();
    }
    auto min = data[start_index];
    auto max = min;

    for (std::size_t i = start_index + 1; i <= end_index; i++) {
        min = std::min(min, data[i]);
        max = std::max(max, data[i]);
    }

    // Widening range to always produce indices in [0, n) instead of [0, n].
    double range = max - min + 1;

    constexpr std::uint64_t msb = 1ull << 63;

    // To allow for differentiating between 0 as a value and an empty slot,
    // we set the most significant bit for all values in our robin hood array to 1.
    // We mask it out again when transferring the values back into the original array.
    // Note: Assuming at least one iteration of radix sort was executed,
    // we know that the msb will be consistent across the entire range.
    std::uint64_t msb_mask = (min & msb) ? ~0ull : ~msb;

    for (std::size_t i = start_index; i <= end_index; i++) {
        auto new_val = data[i];
        auto idx = std::min<std::size_t>((new_val - min) / range * slots, slots - 1);
        auto curr_val = robin_hood_auxiliary[idx];
        new_val |= msb;
        if (curr_val) {
            if (curr_val > new_val && idx > 0) {
                idx--;
                while (idx > 0 && (curr_val = robin_hood_auxiliary[idx]) && curr_val > new_val) {
                    idx--;
                }
            } else if (curr_val < new_val && idx < slots - 1) {
                idx++;
                while (idx < slots - 1 && (curr_val = robin_hood_auxiliary[idx]) && curr_val < new_val) {
                    idx++;
                }
            }
        }
        curr_val = robin_hood_auxiliary[idx];
        if (curr_val) {
            // Either we encountered larger/equal values or we reached the end, either way we need to shift.

            // We need to either shift the larger values right (if they exist) or the smaller values left.
            // We shift into the direction with more space to avoid a situation where we cannot shift further.
            container::element_type shift_val;
            auto shift_idx = idx;
            if (idx > slots / 2) {
                // Shift left.
                if (curr_val > new_val) {
                    idx--;
                    curr_val = robin_hood_auxiliary[idx];
                    shift_idx = idx;
                }
                do {
                    shift_val = robin_hood_auxiliary[shift_idx - 1];
                    robin_hood_auxiliary[shift_idx - 1] = curr_val;
                    curr_val = shift_val;
                    shift_idx--;
                } while (shift_val);
            } else {
                // Shift right.
                if (curr_val < new_val) {
                    idx++;
                    curr_val = robin_hood_auxiliary[idx];
                    shift_idx = idx;
                }
                do {
                    shift_val = robin_hood_auxiliary[shift_idx + 1];
                    robin_hood_auxiliary[shift_idx + 1] = curr_val;
                    curr_val = shift_val;
                    shift_idx++;
                } while (shift_val);
            }
        }

        robin_hood_auxiliary[idx] = new_val;
    }

    auto data_idx = start_index;
    for (std::size_t i = 0; i < slots; i++) {
        if (robin_hood_auxiliary[i]) {
            data[data_idx++] = robin_hood_auxiliary[i] & msb_mask;
            robin_hood_auxiliary[i] = 0;
        }
    }
}

}  // namespace ae
