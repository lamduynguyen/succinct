#pragma once

#include <chrono>
#include <string>

namespace succinct {
namespace detail {

struct timer {
  timer() : m_tick(std::chrono::steady_clock::now()), m_done(false) {}

  bool done() const { return m_done; }

  void report(double &elapsed) {
    auto now = std::chrono::steady_clock::now();
    elapsed  = std::chrono::duration<double, std::micro>(now - m_tick).count();
    m_done   = true;
  }

  const std::string m_msg{};
  std::chrono::steady_clock::time_point m_tick;
  bool m_done;
};

}  // namespace detail
}  // namespace succinct

#define SUCCINCT_TIMEIT(elapsed)                                                       \
  for (::succinct::detail::timer SUCCINCT_TIMEIT_timer; !SUCCINCT_TIMEIT_timer.done(); \
       SUCCINCT_TIMEIT_timer.report(elapsed))                                          \
  /**/
