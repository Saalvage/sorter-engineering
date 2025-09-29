#include "../src/container.hpp"
#include "../src/sorter.hpp"

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <string_view>
#include <vector>

namespace {

auto generate_uniform(std::size_t n) {
  std::vector<std::uint64_t> input(n);

  std::mt19937 gen(n);
  std::uniform_int_distribution<std::uint64_t> dist;
  std::ranges::generate(input, [&] { return dist(gen); });

  return input;
}

void runExperiment(std::string_view name,
                   auto container_factory,
                   auto sort_func,
                   int argc, char **argv) {
  std::size_t n = 1e6;
  std::size_t num_threads = 1;

  if (argc == 3) {
    n = std::stol(argv[1]);
    num_threads = std::stoi(argv[2]);
  } else {
    // The number of threads is just here in case you want to parallelize your code.
    // It's not currently used.
    std::cerr << "Number of threads not specified!\n";
    std::cerr << "Usage: " << argv[0] << " <n> <num_threads>\n";
    return;
  }

  const auto input = generate_uniform(n);

  const auto solution = [&] {
    auto v = input;
    std::ranges::sort(v);
    return v;
  }();

  int iterations = 0;
  std::uint64_t totalNanoseconds = 0;
  std::uint64_t totalNanosecondsFactory = 0;
  int maxIterations = 10'000;

  while (totalNanoseconds < 1000ull * 1000 * 1000 * 20) {
    if (iterations >= maxIterations) {
      break;
    }

    std::chrono::steady_clock::time_point ctor = std::chrono::steady_clock::now();
    auto to_sort = container_factory(input);
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    sort_func(to_sort, num_threads);
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    totalNanoseconds +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(end - begin)
            .count();
    totalNanosecondsFactory +=
        std::chrono::duration_cast<std::chrono::nanoseconds>(begin - ctor)
            .count();
    iterations++;

    if (not std::ranges::equal(to_sort.to_view(), solution)) {
      std::cerr << "Output of " << name << " is not correct!\n";
      return;
    }
  }

  std::cout << "RESULT"
            << " name=" << name << " n=" << n << " t=" << num_threads
            << " iterations=" << iterations
            << " durationNanoseconds=" << totalNanoseconds / iterations
            << " totalDurationNanoseconds=" << totalNanoseconds
            << " constructorNanoseconds=" << totalNanosecondsFactory / iterations
            << " totalConstructorNanoseconds=" << totalNanosecondsFactory
            << '\n';
}

}  // unnamed namespace

int main(int argc, char **argv) {
  runExperiment("sort",
                [](const auto& data) {
                  return ae::container(data);
                },
                [](ae::container& data, int num_threads) {
                  ae::sorter{}.sort(data, num_threads);
                }, argc, argv);

  return 0;
}
