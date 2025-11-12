#define BOOST_TEST_MODULE darray
#include "test_common.hpp"
#include "test_rank_select_common.hpp"

#include "darray.hpp"
#include "mapper.hpp"

#include <cstdlib>

void test_darray(std::vector<bool> const &v) {
  succinct::bit_vector bv(v);
  succinct::darray1 d1(bv);
  succinct::darray0 d0(bv);

  size_t cur_rank  = 0;
  size_t cur_rank0 = 0;
  for (size_t i = 0; i < v.size(); ++i) {
    if (v[i]) {
      ASSERT_EQ(i, d1.select(bv, cur_rank));
      cur_rank += 1;
    } else {
      ASSERT_EQ(i, d0.select(bv, cur_rank0));
      cur_rank0 += 1;
    }
  }

  ASSERT_EQ(cur_rank, d1.num_positions());
  ASSERT_EQ(cur_rank0, d0.num_positions());
}

TEST(test_darray, darray) {
  srand(42);
  size_t N = 10000;

  {
    // Random bitmap
    std::vector<bool> v = random_bit_vector(N);
    test_darray(v);
  }

  {
    // Empty bitmap
    std::vector<bool> v;
    test_darray(v);
  }

  {
    // Only one value
    std::vector<bool> v(N);
    v[37] = 1;
    test_darray(v);
  }

  {
    // Full bitmap
    std::vector<bool> v(N, 1);
    test_darray(v);
  }

  {
    // Very sparse random bitmap
    size_t bigN = (1 << 16) * 4;
    std::vector<bool> v(bigN);
    size_t cur_pos = 0;
    while (cur_pos < bigN) {
      v[cur_pos] = 1;
      cur_pos += rand() % 1024;
    }
    test_darray(v);
  }
}
