#include "test_common.hpp"

#include "bp_vector.hpp"
#include "mapper.hpp"
#include "test_bp_vector_common.hpp"

#include <cstdlib>

template <class BPVector>
void test_parentheses(std::vector<char> const &v, BPVector const &bitmap) {
  std::stack<size_t> stack;
  std::vector<uint64_t> open(v.size());
  std::vector<uint64_t> close(v.size());
  std::vector<uint64_t> enclose(v.size(), uint64_t(-1));

  for (size_t i = 0; i < v.size(); ++i) {
    if (v[i]) {  // opening
      if (!stack.empty()) { enclose[i] = stack.top(); }
      stack.push(i);
    } else {                        // closing
      ASSERT_TRUE(!stack.empty());  // this is more a test on the test
      size_t opening = stack.top();
      stack.pop();
      close[opening] = i;
      open[i]        = opening;
    }
  }
  ASSERT_EQ(0U, stack.size());  // ditto as above

  for (size_t i = 0; i < bitmap.size(); ++i) {
    if (v[i]) {  // opening
      if (enclose[i] != uint64_t(-1)) { ASSERT_EQ(enclose[i], bitmap.enclose(i)); }
      ASSERT_EQ(close[i], bitmap.find_close(i));
    } else {  // closing
      ASSERT_EQ(open[i], bitmap.find_open(i));
    }
  }
}

TEST(bp_vector, test) {
  srand(42);

  {
    std::vector<char> v;
    succinct::bp_vector bitmap(v);
    test_parentheses(v, bitmap);
  }

  {
    std::vector<char> v;
    succinct::random_bp(v, 100000);
    succinct::bp_vector bitmap(v);
    test_parentheses(v, bitmap);
  }

  {
    size_t sizes[] = {2, 4, 512, 514, 8190, 8192, 8194, 16384, 16386, 100000};
    for (size_t i = 0; i < sizeof(sizes) / sizeof(sizes[0]); ++i) {
      std::vector<char> v;
      succinct::random_binary_tree(v, sizes[i]);
      succinct::bp_vector bitmap(v);
      test_parentheses(v, bitmap);
    }
  }

  {
    size_t sizes[]      = {2, 4, 512, 514, 8190, 8192, 8194, 16384, 16386, 32768, 32770};
    size_t iterations[] = {1, 2, 3};
    for (size_t s = 0; s < sizeof(sizes) / sizeof(sizes[0]); ++s) {
      for (size_t r = 0; r < sizeof(iterations) / sizeof(iterations[0]); ++r) {
        std::vector<char> v;
        for (size_t i = 0; i < iterations[r]; ++i) { succinct::bp_path(v, sizes[s]); }
        succinct::bp_vector bitmap(v);
        test_parentheses(v, bitmap);
      }
    }
  }
}
