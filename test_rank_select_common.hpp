#pragma once

#include "test_common.hpp"

template <class Vector>
inline void test_equal_bits(std::vector<bool> const &v, Vector const &bitmap) {
  ASSERT_EQ(v.size(), bitmap.size());
  for (size_t i = 0; i < v.size(); ++i) { ASSERT_EQ((bool)v[i], bitmap[i]); }
}

template <class Vector>
void test_rank_select0(std::vector<bool> const &v, Vector const &bitmap) {
  uint64_t cur_rank0 = 0;
  uint64_t last_zero = uint64_t(-1);

  for (size_t i = 0; i < v.size(); ++i) {
    ASSERT_EQ(cur_rank0, bitmap.rank0(i));
    if (!v[i]) {
      last_zero = i;
      ASSERT_EQ(last_zero, bitmap.select0(cur_rank0));
      ++cur_rank0;
    }
    if (last_zero != uint64_t(-1)) { ASSERT_EQ(last_zero, bitmap.predecessor0(i)); }
  }

  last_zero = uint64_t(-1);
  for (size_t i = v.size() - 1; i + 1 > 0; --i) {
    if (!v[i]) { last_zero = i; }

    if (last_zero != uint64_t(-1)) { ASSERT_EQ(last_zero, bitmap.successor0(i)); }
  }
}

template <class Vector>
void test_rank_select1(std::vector<bool> const &v, Vector const &bitmap) {
  uint64_t cur_rank = 0;
  uint64_t last_one = uint64_t(-1);

  for (size_t i = 0; i < v.size(); ++i) {
    ASSERT_EQ(cur_rank, bitmap.rank(i));

    if (v[i]) {
      last_one = i;
      ASSERT_EQ(last_one, bitmap.select(cur_rank));
      ++cur_rank;
    }

    if (last_one != uint64_t(-1)) { ASSERT_EQ(last_one, bitmap.predecessor1(i)); }
  }

  last_one = uint64_t(-1);
  for (size_t i = v.size() - 1; i + 1 > 0; --i) {
    if (v[i]) { last_one = i; }

    if (last_one != uint64_t(-1)) { ASSERT_EQ(last_one, bitmap.successor1(i)); }
  }
}

template <class Vector>
void test_rank_select(std::vector<bool> const &v, Vector const &bitmap) {
  test_rank_select0(v, bitmap);
  test_rank_select1(v, bitmap);
}

template <class Vector>
void test_delta(Vector const &bitmap) {
  for (size_t i = 0; i < bitmap.num_ones(); ++i) {
    if (i) {
      ASSERT_EQ(bitmap.select(i) - bitmap.select(i - 1), bitmap.delta(i));
    } else {
      ASSERT_EQ(bitmap.select(i), bitmap.delta(i));
    }
  }
}

template <class Vector>
void test_select_enumeration(std::vector<bool> const &v, Vector const &bitmap) {
  // XXX test other starting points
  typename Vector::select_enumerator it(bitmap, 0);

  for (size_t i = 0; i < v.size(); ++i) {
    if (v[i]) {
      uint64_t res = it.next();
      ASSERT_EQ(i, res);
    }
  }
}
