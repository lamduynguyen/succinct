#include <cassert>
#include <cstdint>
#include <iostream>
#include <random>
#include <string>
#include <tuple>
#include <unordered_set>
#include <vector>

#include "elias_fano.hpp"
#include "mapper.hpp"
#include "perftest_common.hpp"
#include "util.hpp"

struct monotone_generator {
  monotone_generator(uint64_t m, uint8_t bits, unsigned int seed) : m_gen(seed), m_bits(bits) {
    m_stack.push_back(state_t{0, m, 0});
  }

  uint64_t next() {
    uint64_t cur_word, cur_m;
    uint8_t cur_depth;

    assert(!m_stack.empty());
    std::tie(cur_word, cur_m, cur_depth) = m_stack.back();
    m_stack.pop_back();

    while (cur_depth < m_bits) {
      std::uniform_int_distribution<uint64_t> dist(0, cur_m);
      uint64_t left_m  = dist(m_gen);
      uint64_t right_m = cur_m - left_m;

      // push left and right children, if present
      if (right_m > 0) {
        m_stack.push_back(state_t{cur_word | (uint64_t(1) << (m_bits - cur_depth - 1)), right_m, cur_depth + 1});
      }
      if (left_m > 0) { m_stack.push_back(state_t{cur_word, left_m, cur_depth + 1}); }

      // pop next child in visit
      std::tie(cur_word, cur_m, cur_depth) = m_stack.back();
      m_stack.pop_back();
    }

    if (cur_m > 1) {
      // push back the current leaf, with cur_m decreased by one
      m_stack.push_back(state_t{cur_word, cur_m - 1, cur_depth});
    }

    return cur_word;
  }

  bool done() const { return m_stack.empty(); }

 private:
  using state_t = std::tuple<uint64_t, uint64_t, uint8_t>;
  std::vector<state_t> m_stack;
  std::mt19937 m_gen;
  uint8_t m_bits;
};

void ef_enumeration_benchmark(uint64_t m, uint8_t bits) {
  succinct::elias_fano::elias_fano_builder bvb(uint64_t(1) << bits, m);
  monotone_generator mgen(m, bits, 37);
  for (size_t i = 0; i < m; ++i) { bvb.push_back(mgen.next()); }
  assert(mgen.done());

  succinct::elias_fano ef(&bvb);
  succinct::mapper::size_tree_of(ef)->dump();

  double elapsed;
  uint64_t foo = 0;
  SUCCINCT_TIMEIT(elapsed) {
    succinct::elias_fano::select_enumerator it(ef, 0);
    for (size_t i = 0; i < m; ++i) { foo ^= it.next(); }
  }
  volatile uint64_t vfoo = foo;
  (void)vfoo;  // silence warning

  std::cerr << "Elapsed: " << elapsed / 1000 << " msec\n" << double(m) / elapsed << " Mcodes/s" << std::endl;
}

void hashtable_enumeration_benchmark(uint64_t m, uint8_t bits) {
  monotone_generator mgen(m, bits, 37);

  std::unordered_set<uint64_t> ht;
  ht.reserve(m);

  for (size_t i = 0; i < m; ++i) { ht.insert(mgen.next()); }
  assert(mgen.done());

  double elapsed;
  uint64_t foo = 0;

  SUCCINCT_TIMEIT(elapsed) {
    for (const auto &v : ht) { foo ^= v; }
  }

  volatile uint64_t vfoo = foo;
  (void)vfoo;

  std::cerr << "Hashtable elapsed: " << elapsed / 1000 << " msec\n" << double(m) / elapsed << " Mcodes/s" << std::endl;
}

void ef_construction_benchmark(uint64_t m, uint8_t bits) {
  monotone_generator mgen(m, bits, 37);

  double elapsed;

  SUCCINCT_TIMEIT(elapsed) {
    succinct::elias_fano::elias_fano_builder bvb(uint64_t(1) << bits, m);
    for (size_t i = 0; i < m; ++i) { bvb.push_back(mgen.next()); }
    succinct::elias_fano ef(&bvb);
  }

  std::cerr << "EF construction elapsed: " << elapsed / 1000 << " msec" << std::endl;
}

void hashtable_construction_benchmark(uint64_t m, uint8_t bits) {
  monotone_generator mgen(m, bits, 37);

  double elapsed;

  SUCCINCT_TIMEIT(elapsed) {
    std::unordered_set<uint64_t> ht;
    ht.reserve(m);
    for (size_t i = 0; i < m; ++i) { ht.insert(mgen.next()); }
  }

  std::cerr << "Hashtable construction elapsed: " << elapsed / 1000 << " msec" << std::endl;
}

std::vector<uint64_t> make_random_indices(uint64_t m, size_t num, unsigned seed = 123) {
  std::mt19937_64 gen(seed);
  std::uniform_int_distribution<uint64_t> dist(0, m - 1);

  std::vector<uint64_t> idx(num);
  for (size_t i = 0; i < num; ++i) { idx[i] = dist(gen); }
  return idx;
}

void ef_random_access_benchmark(uint64_t m, uint8_t bits, size_t num_queries) {
  succinct::elias_fano::elias_fano_builder bvb(uint64_t(1) << bits, m);
  monotone_generator mgen(m, bits, 37);

  std::vector<uint64_t> values(m);
  for (size_t i = 0; i < m; ++i) {
    values[i] = mgen.next();
    bvb.push_back(values[i]);
  }
  succinct::elias_fano ef(&bvb);

  double elapsed;
  uint64_t foo = 0;
  auto queries = make_random_indices(m, num_queries);
  SUCCINCT_TIMEIT(elapsed) {
    for (auto q : queries) { foo ^= ef.select(q); }
  }
  volatile uint64_t vfoo = foo;
  (void)vfoo;
  std::cerr << "EF random access elapsed: " << elapsed / 1000 << " msec\n"
            << double(num_queries) / elapsed << " Mops/s" << std::endl;
}

void hashtable_random_access_benchmark(uint64_t m, uint8_t bits, size_t num_queries) {
  monotone_generator mgen(m, bits, 37);

  std::vector<uint64_t> values(m);
  std::unordered_set<uint64_t> ht;
  ht.reserve(m);

  for (size_t i = 0; i < m; ++i) {
    values[i] = mgen.next();
    ht.insert(values[i]);
  }

  double elapsed;
  uint64_t foo = 0;
  auto queries = make_random_indices(m, num_queries);
  SUCCINCT_TIMEIT(elapsed) {
    for (auto q : queries) {
      auto it = ht.find(values[q]);
      if (it != ht.end()) foo ^= *it;
    }
  }

  volatile uint64_t vfoo = foo;
  (void)vfoo;

  std::cerr << "Hashtable random access elapsed: " << elapsed / 1000 << " msec\n"
            << double(num_queries) / elapsed << " Mops/s" << std::endl;
}

int main(int argc, char **argv) {
  if (argc != 3) {
    std::cerr << "Usage: " << argv[0] << " <m> <bits>\n";
    std::terminate();
  }

  uint64_t m   = std::stoull(argv[1]);
  uint8_t bits = static_cast<uint8_t>(std::stoi(argv[2]));

  std::cerr << "=== Construction ===\n";
  ef_construction_benchmark(m, bits);
  hashtable_construction_benchmark(m, bits);

  std::cerr << "\n=== Scan ===\n";
  ef_enumeration_benchmark(m, bits);
  hashtable_enumeration_benchmark(m, bits);

  std::cerr << "\n=== Random access ===\n";
  ef_random_access_benchmark(m, bits, m);
  hashtable_random_access_benchmark(m, bits, m);
}
