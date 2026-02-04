#pragma once

#include <ranges>

#include "elias_fano.hpp"

namespace succinct {

struct elias_fano_compressed_list {
  using value_type = uint64_t;

  elias_fano_compressed_list() = default;

  template <typename Range>
  elias_fano_compressed_list(Range const &ints) {
    size_t s = 0;
    size_t n = 0;
    for (auto iter = std::begin(ints); iter != std::end(ints); ++iter) {
      s += broadword::msb(*iter + 1);
      n += 1;
    }

    elias_fano::elias_fano_builder ef_builder(s + 1, n + 1);
    bit_vector_builder bits_builder;

    ef_builder.push_back(bits_builder.size());
    for (auto iter = std::begin(ints); iter != std::end(ints); ++iter) {
      size_t val = *iter + 1;
      size_t l   = broadword::msb(val);
      bits_builder.append_bits(val ^ (uint64_t(1) << l), l);
      ef_builder.push_back(bits_builder.size());
    }
    elias_fano(&ef_builder, false).swap(m_ef);
    bit_vector(&bits_builder).swap(m_bits);
  }

  elias_fano_compressed_list(elias_fano::elias_fano_builder &ef_builder, bit_vector_builder &bits_builder) {
    elias_fano(&ef_builder, false).swap(m_ef);
    bit_vector(&bits_builder).swap(m_bits);
  }

  value_type operator[](size_t idx) const {
    std::pair<size_t, size_t> r = m_ef.select_range(idx);
    size_t l                    = r.second - r.first;
    return ((uint64_t(1) << l) | m_bits.get_bits(r.first, l)) - 1;
  }

  size_t size() const { return m_ef.num_ones() - 1; }

  void swap(elias_fano_compressed_list &other) {
    m_ef.swap(other.m_ef);
    m_bits.swap(other.m_bits);
  }

  template <typename Visitor>
  void map(Visitor &visit) {
    visit(m_ef, "m_ef")(m_bits, "m_bits");
  }

 private:
  elias_fano m_ef;
  bit_vector m_bits;
};

}  // namespace succinct
