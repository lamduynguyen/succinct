#include "test_common.hpp"

#include <cstdlib>

#include "elias_fano_compressed_list.hpp"

TEST(test_elias_fano_compressed_list, basic) {
  srand(42);
  const size_t test_size = 12345;

  std::vector<uint64_t> v;

  for (size_t i = 0; i < test_size; ++i) {
    if (rand() < (RAND_MAX / 3)) {
      v.push_back(0);
    } else {
      v.push_back(size_t(rand()));
    }
  }

  succinct::elias_fano_compressed_list vv(v);

  ASSERT_EQ(v.size(), vv.size());
  for (size_t i = 0; i < v.size(); ++i) { ASSERT_EQ(v[i], vv[i]); }
}
