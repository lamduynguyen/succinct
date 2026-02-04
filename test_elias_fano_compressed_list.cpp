#include "broadword.hpp"
#include "elias_fano_compressed_list.hpp"
#include "test_common.hpp"

#include <cstdlib>

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

TEST(test_elias_fano_compressed_list, streaming) {
  srand(42);
  const size_t test_size = 12345;

  std::vector<uint64_t> v;
  auto sum = 0UL;

  for (size_t i = 0; i < test_size; ++i) {
    if (rand() < (RAND_MAX / 3)) {
      v.push_back(0);
    } else {
      v.push_back(size_t(rand()));
    }
    sum += succinct::broadword::msb(v.back() + 1);
  }
  // Start streaming data, assuming metadata (i.e., sum and number of elements)
  succinct::elias_fano::elias_fano_builder ef_builder(sum + 1, test_size + 1);
  succinct::bit_vector_builder bits_builder;
  ef_builder.push_back(bits_builder.size());
  for (auto &val : v) {
    size_t l = succinct::broadword::msb(val + 1);
    bits_builder.append_bits((val + 1) ^ (uint64_t(1) << l), l);
    ef_builder.push_back(bits_builder.size());
  }

  // Construct real elias-fano from the output
  succinct::elias_fano_compressed_list vv(ef_builder, bits_builder);
  ASSERT_EQ(v.size(), vv.size());
  for (size_t i = 0; i < v.size(); ++i) { ASSERT_EQ(v[i], vv[i]); }
}
