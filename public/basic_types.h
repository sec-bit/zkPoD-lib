#pragma once

#include <array>
#include <vector>

// typedef std::array<uint8_t, 32> h256_t;

struct h256_t : public std::array<uint8_t, 32> {
  typedef std::array<uint8_t, 32> Base;
  enum { size_value = std::tuple_size<Base>::value };
  Base const& to_array() const { return *this; }
  Base& to_array() { return *this; }
  static h256_t const& from_array(Base const& base) {
    return *static_cast<h256_t const*>(&base);
  }
  static h256_t& from_array(Base& base) {
    return *static_cast<h256_t*>(&base);
  }
};

static_assert(sizeof(h256_t) == sizeof(h256_t::Base), "");

struct Range {
  Range(uint64_t start = 0, uint64_t count = 0) : start(start), count(count) {}
  uint64_t start;
  uint64_t count;
};

inline bool operator==(Range const& a, Range const& b) {
  return a.start == b.start && a.count == b.count;
}

inline bool operator!=(Range const& a, Range const& b) {
  return a.start != b.start || a.count != b.count;
}
