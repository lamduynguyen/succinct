#include "test_common.hpp"

#include <filesystem>

#include "mapper.hpp"

TEST(test_mapper, basic_map) {
  succinct::mapper::mappable_vector<int> vec;
  ASSERT_EQ(vec.size(), 0U);

  int nums[] = {1, 2, 3, 4};
  vec.assign(nums);

  ASSERT_EQ(4U, vec.size());
  ASSERT_EQ(1, vec[0]);
  ASSERT_EQ(4, vec[3]);
}

class complex_struct {
 public:
  complex_struct() : m_a(0) {}

  void init() {
    m_a          = 42;
    uint32_t b[] = {1, 2};
    m_b.assign(b);
  }

  template <typename Visitor>
  void map(Visitor &visit) {
    visit(m_a, "m_a")(m_b, "m_b");
  }

  uint64_t m_a;
  succinct::mapper::mappable_vector<uint32_t> m_b;
};

TEST(test_mapper, complex_struct_map) {
  complex_struct s;
  s.init();
  succinct::mapper::freeze(s, "temp.bin");

  ASSERT_EQ(24, succinct::mapper::size_of(s));

  complex_struct mapped_s;
  ASSERT_EQ(0, mapped_s.m_a);
  ASSERT_EQ(0U, mapped_s.m_b.size());
}
