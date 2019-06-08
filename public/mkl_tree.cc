#include "mkl_tree.h"

#include <cryptopp/sha.h>
#include <cassert>
#include "tick.h"
#include "public.h"
#include "misc.h"

namespace mkl {
h256_t kEmptyH256 = h256_t();

void TwoToOne(h256_t const& a, h256_t const& b, h256_t* r) {
  CryptoPP::Keccak_256 hash;
  hash.Update(a.data(), 32);
  hash.Update(b.data(), 32);
  hash.Final(r->data());
};

// return mkl root
h256_t CalcPath(GetItem get_item, uint64_t item_count, uint64_t leaf,
                Path* path) {
  auto get_item_or_empty = [&get_item, item_count](uint64_t i) {
    if (i < item_count) return get_item(i);
    return kEmptyH256;
  };

  struct H {
    uint64_t pos;
    h256_t item;
    int height;
    bool flag;
  };
  std::vector<H> s;
  s.reserve(256);
  uint64_t offset;

  uint64_t count = misc::Pow2UB(item_count);
  assert((count & (count - 1)) == 0);

  offset = 0;
  for (;;) {
    if (s.size() >= 2) {
      auto& right = s[s.size() - 1];
      auto& left = s[s.size() - 2];
      if (right.height == left.height) {
        bool flag;
        if (right.flag) {
          path->push_back(left.item);
          flag = true;
        } else if (left.flag) {
          path->push_back(right.item);
          flag = true;
        } else {
          flag = false;
        }
        TwoToOne(left.item, right.item, &left.item);
        left.height += 1;
        left.flag = flag;
        s.pop_back();
        continue;
      }
    }

    if (offset == count) {
      assert(s.size() == 1);
      break;
    }

    s.resize(s.size() + 1);

    // push new leaf
    auto& last_s = s[s.size() - 1];
    last_s.pos = offset;
    last_s.flag = (offset == leaf);
    last_s.item = get_item_or_empty(offset++);
    last_s.height = 0;
  }

  return s[0].item;
}

bool CalcRoot(std::string const& file, h256_t* root) {
  try {
    io::mapped_file_params params;
    params.path = file;
    params.flags = io::mapped_file_base::readonly;
    io::mapped_file_source view(params);
    auto start = (uint8_t*)view.data();
    if (!view.size() || view.size() % 32) return false;
    uint64_t n = view.size() / 32;
    auto get_item = [start, n](uint64_t i) -> h256_t {
      assert(i < n);
      h256_t h;
      memcpy(h.data(), start + i * 32, 32);
      return h;
    };
    *root = mkl::CalcRoot(get_item, n);
    return true;
  } catch (std::exception&) {
    return false;
  }
}

h256_t CalcRoot(GetItem get_item, uint64_t item_count) {
  // Tick tick(std::string(__FUNCTION__) + ", item_count: " +
  //          std::to_string(item_count));
  uint64_t count = misc::Pow2UB(item_count);
  assert((count & (count - 1)) == 0);

  auto get_item_or_empty = [&get_item, item_count](uint64_t i) {
    if (i < item_count) return get_item(i);
    return kEmptyH256;
  };

  typedef std::pair<h256_t, int> H;  // pair<item, height>
  std::vector<H> s;
  s.reserve(256);

  uint64_t offset = 0;
  for (;;) {
    if (s.size() >= 2) {
      auto& right = s[s.size() - 1];
      auto& left = s[s.size() - 2];
      if (right.second == left.second) {
        TwoToOne(left.first, right.first, &left.first);
        ++left.second;
        s.pop_back();
        continue;
      }
    }

    if (offset == count) {
      assert(s.size() == 1);
      assert(((uint64_t)1 << s[0].second) == count);
      break;
    }

    s.resize(s.size() + 1);

    // push new leaf
    s[s.size() - 1].first = get_item_or_empty(offset++);
    s[s.size() - 1].second = 0;
  }
  return s[0].first;
}

bool VerifyPath(uint64_t pos, h256_t value, uint64_t count, h256_t const& root,
                Path const& path) {
  // Tick tick(__FUNCTION__);
  auto depth = misc::Log2UB(count);
  assert(path.size() == depth);
  if (path.size() != depth) return false;

  for (uint64_t i = 0; i < depth; ++i) {
    if (pos % 2) {
      TwoToOne(path[i], value, &value);
    } else {
      TwoToOne(value, path[i], &value);
    }
    pos /= 2;
  }

  assert(value == root);
  return value == root;
}

bool VerifyRangePath(GetItem get_item, Range const& range, uint64_t item_count,
                     h256_t const& root, Path const& path) {
  assert(range.start < item_count);
  if (range.start >= item_count) return false;
  assert(range.count && range.count <= item_count);
  if (!range.count || range.count > item_count) return false;
  assert(range.start + range.count <= item_count);
  if (range.start + range.count > item_count) return false;

  if (item_count == 1) {
    assert(range.start == 0 && range.count == 1 && path.empty());
    if (range.start || range.count != 1 || !path.empty()) return false;
    return (root == get_item(0));
  }

  uint64_t depth = misc::Log2UB(item_count);
  auto ranges = SplitRange(range);
  uint64_t path_count = 0;
  for (auto& i : ranges) {
    assert((i.count & (i.count - 1)) == 0);
    if ((i.count & (i.count - 1))) return false;
    assert(i.start % i.count == 0);
    if (i.start % i.count) return false;
    uint64_t level = misc::Log2UB(i.count);
    path_count += depth - level;
  }
  assert(path_count == path.size());
  if (path_count != path.size()) return false;

  h256_t const* path_leaf = &path[0];
  for (auto& r : ranges) {
    uint64_t leaf = r.start / r.count;
    uint64_t level = misc::Log2UB(r.count);

    auto get_range_item = [&get_item, item_count, &r](uint64_t i) {
      auto offset = i + r.start;
      if (offset >= item_count) return kEmptyH256;
      return get_item(offset);
    };

    h256_t leaf_root = CalcRoot(std::move(get_range_item), r.count);

    for (uint64_t i = level; i < depth; ++i) {
      if (leaf % 2 == 0) {
        TwoToOne(leaf_root, *path_leaf, &leaf_root);
      } else {
        TwoToOne(*path_leaf, leaf_root, &leaf_root);
      }
      leaf /= 2;
      ++path_leaf;
    }
    assert(leaf_root == root);
    if (leaf_root != root) return false;
  }
  return true;
}

std::vector<Range> SplitRange(Range const& range) {
  std::vector<Range> ret;
  auto start = range.start;
  auto count = range.count;
  uint64_t max_level = misc::Log2UB(count);

  for (; count;) {
    Range item;
    item.start = start;
    uint64_t distance;
    for (int64_t i = (int64_t)max_level; i >= 0; --i) {
      distance = 1ULL << i;
      if (start % distance) continue;
      while (distance > count) {
        distance /= 2;
      }
      item.count = distance;
      break;
    }
    ret.push_back(item);
    start += item.count;
    count -= item.count;
  }
  return ret;
}

Tree BuildTree(uint64_t item_count, GetItem const& get_item) {
  std::vector<h256_t> digests;
  if (item_count == 1) {
    digests.push_back(get_item(0));
    return digests;
  }

  auto align_count = misc::Pow2UB(item_count);
  auto depth = misc::Log2UB(item_count);
  digests.reserve(align_count - 1);

  auto get_item_or_empty = [item_count, &get_item](uint64_t i) -> h256_t {
    if (i >= item_count) return kEmptyH256;
    return get_item(i);
  };

  for (uint64_t i = 0; i < align_count / 2; ++i) {
    auto left = get_item_or_empty(i * 2);
    auto right = get_item_or_empty(i * 2 + 1);
    digests.resize(digests.size() + 1);
    TwoToOne(left, right, &digests.back());
  }

  for (uint64_t i = 1; i < depth; ++i) {
    uint64_t length = 1ULL << (depth - i);
    uint64_t offset = digests.size() - length;
    for (uint64_t j = 0; j < length / 2; ++j) {
      digests.resize(digests.size() + 1);
      TwoToOne(digests[offset + j * 2], digests[offset + j * 2 + 1],
               &digests.back());
    }
  }

  assert(digests.size() == (align_count - 1));
  return digests;
}

size_t GetTreeSize(uint64_t item_count) {
  if (item_count == 1) return 1;
  return misc::Pow2UB(item_count) - 1;
}

Path GetRangePath(uint64_t item_count, GetItem const& get_item,
                  Tree const& tree, Range const& range) {
  std::vector<h256_t> path;
  if (tree.size() != GetTreeSize(item_count))
    throw std::runtime_error("invaild parameters");

  if (tree.size() == 1) return path;  // empty

  auto align_count = misc::Pow2UB(item_count);
  auto depth = misc::Log2UB(item_count);

  auto ranges = SplitRange(range);

  auto get_item_or_empty = [item_count, &get_item](uint64_t i) -> h256_t {
    if (i >= item_count) return kEmptyH256;
    return get_item(i);
  };

  auto get_offset = [align_count](uint64_t level, uint64_t leaf) -> uint64_t {
    uint64_t count = align_count;
    uint64_t offset = 0;
    for (uint64_t i = 0; i < level; ++i) {
      count /= 2;
      offset += count;
    }
    return offset + leaf;
  };

  for (auto& r : ranges) {
    assert(r.start % r.count == 0);
    uint64_t leaf = r.start / r.count;

    assert((r.count & (r.count - 1)) == 0);
    uint64_t level = misc::Log2UB(r.count);

    for (uint64_t i = level; i < depth; ++i) {
      uint64_t brother = (leaf % 2) ? leaf - 1 : leaf + 1;
      if (i == 0) {
        path.push_back(get_item_or_empty(brother));
      } else {
        auto offset = get_offset(i - 1, brother);
        path.push_back(tree[offset]);
      }
      leaf /= 2;
    }
  }

  assert(VerifyRangePath(get_item, range, item_count, tree.back(), path));
  return path;
}

}  // namespace mkl