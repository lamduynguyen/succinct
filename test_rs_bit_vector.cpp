#include "test_common.hpp"
#include "test_rank_select_common.hpp"

#include <cstdlib>

#include "mapper.hpp"
#include "rs_bit_vector.hpp"

TEST(test_rs_bit_vector, basic) {
  srand(42);

  // empty vector
  std::vector<bool> v;
  succinct::rs_bit_vector bitmap;

  succinct::rs_bit_vector(v).swap(bitmap);
  ASSERT_EQ(v.size(), bitmap.size());
  succinct::rs_bit_vector(v, true).swap(bitmap);
  ASSERT_EQ(v.size(), bitmap.size());

  // random vector
  v = random_bit_vector();

  succinct::rs_bit_vector(v).swap(bitmap);
  ASSERT_EQ(v.size(), bitmap.size());
  test_equal_bits(v, bitmap);
  test_rank_select(v, bitmap);

  succinct::rs_bit_vector(v, true, true).swap(bitmap);
  test_rank_select(v, bitmap);

  v.resize(10000);
  v[9999] = 1;
  v[9000] = 1;
  succinct::rs_bit_vector(v).swap(bitmap);

  ASSERT_EQ(v.size(), bitmap.size());
  test_rank_select(v, bitmap);
  succinct::rs_bit_vector(v, true, true).swap(bitmap);
  test_rank_select(v, bitmap);

  // corner cases
  v.clear();
  v.resize(10000);
  v[0]    = 1;
  v[511]  = 1;
  v[512]  = 1;
  v[1024] = 1;
  v[2112] = 1;
  succinct::rs_bit_vector(v).swap(bitmap);

  ASSERT_EQ(v.size(), bitmap.size());
  test_rank_select(v, bitmap);
  succinct::rs_bit_vector(v, true).swap(bitmap);
  test_rank_select(v, bitmap);
}
