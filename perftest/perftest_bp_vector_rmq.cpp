#include <cstdint>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "bp_vector.hpp"
#include "perftest_common.hpp"
#include "test_bp_vector_common.hpp"
#include "util.hpp"

// Compute average excess RMQ time for a BP vector
double time_avg_rmq(const succinct::bp_vector &bp, size_t sample_size = 1000000) {
  using range_pair = std::pair<uint64_t, uint64_t>;
  std::vector<range_pair> pairs_sample;
  pairs_sample.reserve(sample_size);

  // C++ random engine for deterministic sampling
  std::mt19937 rng(42);
  std::uniform_int_distribution<uint64_t> dist(0, bp.size() - 1);

  for (size_t i = 0; i < sample_size; ++i) {
    uint64_t a = dist(rng);
    uint64_t b = a + (dist(rng) % (bp.size() - a));
    pairs_sample.emplace_back(a, b);
  }

  volatile uint64_t foo;  // prevent compiler optimization
  size_t rmq_performed = 0;
  double elapsed;

  SUCCINCT_TIMEIT(elapsed) {
    for (const auto &r : pairs_sample) {
      foo = bp.excess_rmq(r.first, r.second);
      ++rmq_performed;
    }
  }

  (void)foo;  // silence warning
  return elapsed / static_cast<double>(rmq_performed);
}

// Build a random binary tree in a BP vector
void build_random_binary_tree(succinct::bp_vector &bp, size_t size) {
  succinct::bit_vector_builder builder;
  succinct::random_binary_tree(builder, size);
  succinct::bp_vector(&builder, true, false).swap(bp);
}

// Benchmark excess RMQ across multiple tree sizes
void rmq_benchmark(size_t runs) {
  static const size_t sample_size = 10000000;

  std::cout << "SUCCINCT_EXCESS_RMQ\n";
  std::cout << "log_height\texcess_rmq_us\n";

  for (size_t ln = 10; ln <= 28; ln += 2) {
    size_t n       = 1 << ln;
    double elapsed = 0;

    for (size_t run = 0; run < runs; ++run) {
      succinct::bp_vector bp;
      build_random_binary_tree(bp, n);
      elapsed += time_avg_rmq(bp, sample_size);
    }

    std::cout << ln << "\t" << elapsed / static_cast<double>(runs) << "\n";
  }
}

int main(int argc, char **argv) {
  size_t runs = 1;
  if (argc == 2) { runs = std::stoull(argv[1]); }

  rmq_benchmark(runs);
}
