#pragma once

#include <cstdint>
#include <fstream>
#include <list>
#include <stdexcept>
#include <string>

namespace succinct {
namespace util {

inline void trim_newline_chars(std::string &s) {
  size_t l = s.size();
  while (l && (s[l - 1] == '\r' || s[l - 1] == '\n')) { --l; }
  s.resize(l);
}

// this is considerably faster than std::getline
inline bool fast_getline(std::string &line, FILE *input = stdin, bool trim_newline = false) {
  line.clear();
  static const size_t max_buffer = 65536;
  char buffer[max_buffer];
  bool done = false;
  while (!done) {
    if (!fgets(buffer, max_buffer, input)) {
      if (!line.size()) {
        return false;
      } else {
        done = true;
      }
    }
    line += buffer;
    if (*line.rbegin() == '\n') { done = true; }
  }
  if (trim_newline) { trim_newline_chars(line); }
  return true;
}

struct auto_file {
  auto_file(const char *name, const char *mode = "rb") : m_file(0) {
    m_file = fopen(name, mode);
    if (!m_file) {
      std::string msg("Unable to open file '");
      msg += name;
      msg += "'.";
      throw std::invalid_argument(msg);
    }
  }

  ~auto_file() {
    if (m_file) { fclose(m_file); }
  }

  FILE *get() { return m_file; }

 private:
  auto_file();
  auto_file(const auto_file &);
  auto_file &operator=(const auto_file &);

  FILE *m_file;
};

typedef std::pair<const uint8_t *, const uint8_t *> char_range;

struct identity_adaptor {
  char_range operator()(char_range s) const { return s; }
};

struct stl_string_adaptor {
  char_range operator()(std::string const &s) const {
    const uint8_t *buf = reinterpret_cast<const uint8_t *>(s.c_str());
    const uint8_t *end = buf + s.size() + 1;  // add the null terminator
    return char_range(buf, end);
  }
};

struct input_error : std::invalid_argument {
  input_error(std::string const &what) : invalid_argument(what) {}
};

template <typename T>
inline void dispose(T &t) {
  T().swap(t);
}

inline uint64_t int2nat(int64_t x) {
  if (x < 0) {
    return uint64_t(-2 * x - 1);
  } else {
    return uint64_t(2 * x);
  }
}

inline int64_t nat2int(uint64_t n) {
  if (n % 2) {
    return -int64_t((n + 1) / 2);
  } else {
    return int64_t(n / 2);
  }
}

template <typename IntType1, typename IntType2>
inline IntType1 ceil_div(IntType1 dividend, IntType2 divisor) {
  // XXX(ot): put some static check that IntType1 >= IntType2
  IntType1 d = IntType1(divisor);
  return IntType1(dividend + d - 1) / d;
}

}  // namespace util
}  // namespace succinct
