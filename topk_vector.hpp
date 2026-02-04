#pragma once

#include <algorithm>
#include <cassert>
#include <tuple>
#include <vector>

#include "cartesian_tree.hpp"

namespace succinct {

// XXX: currently uses default comparator; arbitrary comparator support can be added
template <typename Vector>
class topk_vector {
 public:
  using vector_type       = Vector;
  using value_type        = typename vector_type::value_type;
  using entry_type        = std::tuple<value_type, uint64_t>;  // value + index
  using entry_vector_type = std::vector<entry_type>;

  topk_vector() = default;

  template <typename Range>
  topk_vector(const Range &v) {
    cartesian_tree(v, std::greater<typename Range::value_type>()).swap(m_cartesian_tree);
    vector_type(v).swap(m_v);
  }

  value_type operator[](uint64_t idx) const { return m_v[idx]; }

  uint64_t size() const { return m_v.size(); }

  class enumerator {
   public:
    enumerator() : m_topkv(nullptr) {}

    bool next() {
      if (m_q.empty()) return false;

      value_type cur_mid_val;
      uint64_t cur_mid, cur_a, cur_b;

      std::pop_heap(m_q.begin(), m_q.end(), value_index_comparator());
      std::tie(cur_mid_val, cur_mid, cur_a, cur_b) = m_q.back();
      m_q.pop_back();

      m_cur = std::make_tuple(cur_mid_val, cur_mid);

      if (cur_mid != cur_a) {
        uint64_t m = m_topkv->m_cartesian_tree.rmq(cur_a, cur_mid - 1);
        m_q.push_back(queue_element_type{m_topkv->m_v[m], m, cur_a, cur_mid - 1});
        std::push_heap(m_q.begin(), m_q.end(), value_index_comparator());
      }

      if (cur_mid != cur_b) {
        uint64_t m = m_topkv->m_cartesian_tree.rmq(cur_mid + 1, cur_b);
        m_q.push_back(queue_element_type{m_topkv->m_v[m], m, cur_mid + 1, cur_b});
        std::push_heap(m_q.begin(), m_q.end(), value_index_comparator());
      }

      return true;
    }

    const entry_type &value() const { return m_cur; }

    void swap(enumerator &other) {
      using std::swap;
      swap(m_topkv, other.m_topkv);
      swap(m_q, other.m_q);
      swap(m_cur, other.m_cur);
    }

   private:
    void set(const topk_vector *topkv, uint64_t a, uint64_t b) {
      assert(a <= b);
      clear();
      m_topkv = topkv;

      uint64_t m = m_topkv->m_cartesian_tree.rmq(a, b);
      m_q.push_back(queue_element_type{m_topkv->m_v[m], m, a, b});
      std::push_heap(m_q.begin(), m_q.end(), value_index_comparator());
    }

    using queue_element_type = std::tuple<value_type, uint64_t, uint64_t, uint64_t>;

    struct value_index_comparator {
      bool operator()(const queue_element_type &a, const queue_element_type &b) const {
        // lexicographic: increasing value, decreasing index
        auto [val_a, idx_a, _, __]     = a;
        auto [val_b, idx_b, ___, ____] = b;
        return (val_a < val_b) || (val_a == val_b && idx_a > idx_b);
      }
    };

   public:
    void clear() {
      m_topkv = nullptr;
      m_q.clear();
    }

   private:
    friend class topk_vector;
    const topk_vector *m_topkv;
    std::vector<queue_element_type> m_q;
    entry_type m_cur;
  };

  // NOTE this is b inclusive
  // XXX switch to [a, b) ?
  void get_topk_enumerator(uint64_t a, uint64_t b, enumerator &ret) const { ret.set(this, a, b); }

  enumerator get_topk_enumerator(uint64_t a, uint64_t b) const {
    enumerator ret;
    get_topk_enumerator(a, b, ret);
    return ret;
  }

  entry_vector_type topk(uint64_t a, uint64_t b, size_t k) const {
    entry_vector_type ret(std::min<size_t>(b - a + 1, k));
    enumerator it = get_topk_enumerator(a, b);

    for (size_t i = 0; i < ret.size(); ++i) {
      bool has_next = it.next();
      assert(has_next);
      (void)has_next;
      ret[i] = it.value();
    }

    assert(ret.size() == k || !it.next());
    return ret;
  }

  template <typename Visitor>
  void map(Visitor &visit) {
    visit(m_v, "m_v")(m_cartesian_tree, "m_cartesian_tree");
  }

  void swap(topk_vector &other) {
    other.m_v.swap(m_v);
    other.m_cartesian_tree.swap(m_cartesian_tree);
  }

 protected:
  vector_type m_v;
  cartesian_tree m_cartesian_tree;
};

}  // namespace succinct
