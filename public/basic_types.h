#pragma once

#include <array>
#include <vector>

#include <boost/lexical_cast.hpp>

// typedef std::array<uint8_t, 32> h256_t;

struct h256_t : public std::array<uint8_t, 32> {
  typedef std::array<uint8_t, 32> Base;
  enum { size_value = std::tuple_size<Base>::value };
  Base const& to_array() const { return *this; }
  Base& to_array() { return *this; }
  static h256_t const& from_array(Base const& base) {
    return *static_cast<h256_t const*>(&base);
  }
  static h256_t& from_array(Base& base) { return *static_cast<h256_t*>(&base); }
};

static_assert(sizeof(h256_t) == sizeof(h256_t::Base), "");

struct Range {
  Range(uint64_t start = 0, uint64_t count = 0) : start(start), count(count) {}
  uint64_t start;
  uint64_t count;

  static std::string to_string(Range const& v) {
    return std::to_string(v.start) + "-" + std::to_string(v.count);
  }
  
  // throw
  static Range from_string(std::string const& s) {
    Range t;
    auto pos = s.find_first_of('-');
    if (pos != std::string::npos) {
      std::string s1 = s.substr(0, pos);
      std::string s2 = s.substr(pos + 1);
      t.start = boost::lexical_cast<uint64_t>(s1);
      t.count = boost::lexical_cast<uint64_t>(s2);
    } else {
      t.start = boost::lexical_cast<uint64_t>(s);
      t.count = 1;
    }
    return t;
  }
};

inline bool operator==(Range const& a, Range const& b) {
  return a.start == b.start && a.count == b.count;
}

inline bool operator!=(Range const& a, Range const& b) {
  return a.start != b.start || a.count != b.count;
}
