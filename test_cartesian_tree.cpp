#define BOOST_TEST_MODULE cartesian_tree
#include "test_common.hpp"

#include <algorithm>
#include <cstdlib>
#include <random>
#include <vector>

#include "cartesian_tree.hpp"
#include "mapper.hpp"

using value_type = uint64_t;

// XXX test (de)serialization
template <typename Comparator>
void test_rmq(const std::vector<value_type> &v, const succinct::cartesian_tree &tree, Comparator comp) {
  ASSERT_EQ(v.size(), tree.size());
  if (v.empty()) return;

  std::vector<uint64_t> tests;
  // A few special cases
  tests.push_back(0);
  tests.push_back(1);

  // Global minimum
  tests.push_back(static_cast<uint64_t>(std::min_element(v.begin(), v.end(), comp) - v.begin()));

  // Random tests
  std::mt19937 rng(42);
  std::uniform_int_distribution<size_t> dist(0, v.size() - 1);
  for (size_t t = 0; t < 10; ++t) { tests.push_back(dist(rng)); }

  for (uint64_t a : tests) {
    if (a >= v.size()) continue;

    uint64_t min_idx   = a;
    value_type cur_min = v[a];
    ASSERT_EQ(min_idx, tree.rmq(a, a));

    for (uint64_t b = a + 1; b < v.size(); ++b) {
      if (comp(v[b], cur_min)) {
        cur_min = v[b];
        min_idx = b;
      }
      ASSERT_EQ(min_idx, tree.rmq(a, b));
    }
  }
}

TEST(test_cartesian_tree, basic) {
  std::mt19937 rng(42);

  // Empty vector
  {
    std::vector<value_type> v;
    succinct::cartesian_tree t(v);
    test_rmq(v, t, std::less<value_type>());
  }

  // Increasing sequence
  {
    std::vector<value_type> v(20000);
    for (size_t i = 0; i < v.size(); ++i) v[i] = i;

    {
      succinct::cartesian_tree t(v);
      test_rmq(v, t, std::less<value_type>());
    }
    {
      succinct::cartesian_tree t(v, std::greater<value_type>());
      test_rmq(v, t, std::greater<value_type>());
    }
  }

  // Peak-shaped vector
  {
    std::vector<value_type> v(20000);
    for (size_t i = 0; i < v.size(); ++i) { v[i] = (i < v.size() / 2) ? i : v.size() - i; }

    {
      succinct::cartesian_tree t(v);
      test_rmq(v, t, std::less<value_type>());
    }
    {
      succinct::cartesian_tree t(v, std::greater<value_type>());
      test_rmq(v, t, std::greater<value_type>());
    }
  }

  // Random vectors of various sizes
  {
    std::vector<size_t> sizes = {2, 4, 512, 514, 8190, 8192, 8194, 16384, 16386, 100000};
    std::mt19937 rng2(42);
    for (size_t n : sizes) {
      std::vector<value_type> v(n);
      std::uniform_int_distribution<value_type> val_dist(0, 1023);
      for (auto &x : v) x = val_dist(rng2);

      succinct::cartesian_tree t(v);
      test_rmq(v, t, std::less<value_type>());
    }
  }
}
