#define BOOST_TEST_MODULE gamma_vector
#include "test_common.hpp"

#include <cstdlib>

#include "gamma_vector.hpp"

TEST(test_gamma_vector, basic) {
  srand(42);
  const size_t test_size = 12345;

  std::vector<uint64_t> v;

  for (size_t i = 0; i < test_size; ++i) {
    if (rand() < (RAND_MAX / 3)) {
      v.push_back(0);
    } else {
      v.push_back(uint64_t(rand()));
    }
  }

  succinct::gamma_vector vv(v);

  ASSERT_EQ(v.size(), vv.size());
  for (size_t i = 0; i < v.size(); ++i) { ASSERT_EQ(v[i], vv[i]); }
}

TEST(test_gamma_vector, gamma_enumerator) {
  srand(42);
  const size_t test_size = 12345;

  std::vector<uint64_t> v;

  for (size_t i = 0; i < test_size; ++i) {
    if (rand() < (RAND_MAX / 3)) {
      v.push_back(0);
    } else {
      v.push_back(uint64_t(rand()));
    }
  }

  succinct::gamma_vector vv(v);

  size_t i   = 0;
  size_t pos = 0;

  succinct::forward_enumerator<succinct::gamma_vector> e(vv, pos);
  while (pos < vv.size()) {
    uint64_t next = e.next();
    ASSERT_EQ(next, v[pos]);
    pos += 1;

    size_t step = uint64_t(rand()) % (vv.size() - pos + 1);
    pos += step;
    e = succinct::forward_enumerator<succinct::gamma_vector>(vv, pos);
    i += 1;
  }
}
