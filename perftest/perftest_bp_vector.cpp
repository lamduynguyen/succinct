#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <vector>

#include "bp_vector.hpp"
#include "mapper.hpp"
#include "perftest_common.hpp"
#include "test_bp_vector_common.hpp"
#include "util.hpp"

// Trait for succinct::bp_vector
struct succinct_bp_vector_traits {
  using builder_type   = succinct::bit_vector_builder;
  using bp_vector_type = succinct::bp_vector;

  static inline void build(builder_type &builder, bp_vector_type &bp) {
    bp_vector_type(&builder, true, false).swap(bp);
  }

  static inline std::string log_header() { return "SUCCINCT"; }

  static inline double bits_per_bp(bp_vector_type &vec) {
    return double(succinct::mapper::size_of(vec)) * 8.0 / double(vec.size());
  }
};

// Measure average time per find_close operation
template <typename BpVector>
double time_visit(const BpVector &bp, size_t sample_size = 1000000) {
  std::vector<char> random_bits;
  random_bits.reserve(sample_size);

  std::mt19937 rng(42);  // deterministic
  std::uniform_int_distribution<int> bit_dist(0, 1);

  for (size_t i = 0; i < sample_size; ++i) { random_bits.push_back(bit_dist(rng)); }

  volatile size_t foo         = 0;  // prevent optimization
  size_t find_close_performed = 0;
  size_t steps_done           = 0;
  double elapsed;

  SUCCINCT_TIMEIT(elapsed) {
    while (steps_done < sample_size) {
      size_t cur_node = 1;  // root

      while (bp[cur_node] && steps_done < sample_size) {
        if (random_bits[steps_done++]) {
          size_t next_node = bp.find_close(cur_node);
          cur_node         = next_node + 1;
          ++find_close_performed;
        } else {
          ++cur_node;
        }
      }
      foo = cur_node;
    }
  }

  (void)foo;  // silence warning
  return elapsed / static_cast<double>(find_close_performed);
}

// Build a random binary tree using the provided BP trait
template <typename BpVectorTraits>
void build_random_binary_tree(typename BpVectorTraits::bp_vector_type &bp, size_t size) {
  typename BpVectorTraits::builder_type builder;
  succinct::random_binary_tree(builder, size);
  BpVectorTraits::build(builder, bp);
}

// Benchmark BP vector operations
template <typename BpVectorTraits>
void bp_benchmark(size_t runs) {
  static const size_t sample_size = 10000000;

  std::cout << BpVectorTraits::log_header() << "\n";
  std::cout << "log_height\tfind_close_us\tbits_per_bp\n";

  for (size_t ln = 10; ln <= 28; ln += 2) {
    size_t n           = 1 << ln;
    double elapsed     = 0;
    double bits_per_bp = 0;

    for (size_t run = 0; run < runs; ++run) {
      typename BpVectorTraits::bp_vector_type bp;
      build_random_binary_tree<BpVectorTraits>(bp, n);
      elapsed += time_visit(bp, sample_size);
      bits_per_bp += BpVectorTraits::bits_per_bp(bp);
    }

    std::cout << ln << "\t" << elapsed / static_cast<double>(runs) << "\t" << bits_per_bp / static_cast<double>(runs)
              << "\n";
  }
}

int main(int argc, char **argv) {
  size_t runs = 1;
  if (argc == 2) { runs = std::stoull(argv[1]); }

  bp_benchmark<succinct_bp_vector_traits>(runs);
}
