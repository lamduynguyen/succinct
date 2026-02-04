#include <cstdint>
#include <cstdlib>
#include <iostream>
#include <random>
#include <utility>
#include <vector>

#include "cartesian_tree.hpp"
#include "perftest_common.hpp"
#include "test_bp_vector_common.hpp"
#include "util.hpp"

// Compute average RMQ time for a Cartesian tree
double time_avg_rmq(const succinct::cartesian_tree &tree, size_t sample_size = 1000000) {
  using range_pair = std::pair<uint64_t, uint64_t>;
  std::vector<range_pair> pairs_sample;
  pairs_sample.reserve(sample_size);

  // Use C++ random engine instead of rand()
  std::mt19937 rng(42);  // deterministic seed
  std::uniform_int_distribution<uint64_t> dist(0, tree.size() - 1);

  for (size_t i = 0; i < sample_size; ++i) {
    uint64_t a = dist(rng);
    uint64_t b = a + (dist(rng) % (tree.size() - a));
    pairs_sample.emplace_back(a, b);
  }

  volatile uint64_t foo;  // prevent compiler optimization

  size_t rmq_performed = 0;
  double elapsed;
  SUCCINCT_TIMEIT(elapsed) {
    for (const auto &r : pairs_sample) {
      foo = tree.rmq(r.first, r.second);
      ++rmq_performed;
    }
  }

  (void)foo;  // silence warning
  return elapsed / static_cast<double>(rmq_performed);
}

// Benchmark RMQ over multiple runs and tree sizes
void rmq_benchmark(size_t runs) {
  static const size_t sample_size = 10000000;

  std::cout << "SUCCINCT_CARTESIAN_TREE_RMQ\n";
  std::cout << "log_height\texcess_rmq_us\n";

  std::mt19937 rng(42);  // deterministic generator
  std::uniform_int_distribution<uint64_t> value_dist(0, 1023);

  for (size_t ln = 10; ln <= 28; ln += 2) {
    size_t n       = 1 << ln;
    double elapsed = 0;

    for (size_t run = 0; run < runs; ++run) {
      std::vector<uint64_t> v(n);
      for (auto &val : v) { val = value_dist(rng); }

      succinct::cartesian_tree tree(v);
      elapsed += time_avg_rmq(tree, sample_size);
    }

    std::cout << ln << "\t" << elapsed / static_cast<double>(runs) << "\n";
  }
}

int main(int argc, char **argv) {
  size_t runs = 1;
  if (argc == 2) { runs = std::stoull(argv[1]); }

  rmq_benchmark(runs);
}
