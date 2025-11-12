#pragma once

#include "gtest/gtest.h"

#include <stdint.h>
#include <stack>
#include <vector>

inline std::vector<bool> random_bit_vector(size_t n = 10000, double density = 0.5) {
  std::vector<bool> v;
  for (size_t i = 0; i < n; ++i) { v.push_back(rand() < (RAND_MAX * density)); }
  return v;
}
