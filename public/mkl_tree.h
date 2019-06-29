#pragma once

#include <stdint.h>
#include <algorithm>
#include <functional>
#include <memory>
#include <vector>

#include "basic_types.h"

namespace mkl {

typedef std::vector<h256_t> Tree;
typedef std::vector<h256_t> Path;

typedef std::function<h256_t(uint64_t)> GetItem;

size_t Log2UB(size_t n);

uint64_t Pow2UB(uint64_t v);

void TwoToOne(h256_t const& a, h256_t const& b, h256_t* r);

h256_t CalcPath(GetItem get_item, uint64_t item_count, uint64_t leaf,
                Path* path);

h256_t CalcRoot(GetItem get_item, uint64_t item_count);

bool CalcRoot(std::string const& file, h256_t* root);

bool VerifyPath(uint64_t pos, h256_t value, uint64_t count, h256_t const& root,
                Path const& path);

std::vector<Range> SplitRange(Range const& range);

bool VerifyRangePath(GetItem get_item, Range const& range, uint64_t item_count,
                     h256_t const& root, Path const& path);

size_t GetTreeSize(uint64_t item_count);

Tree BuildTree(uint64_t item_count, GetItem const& get_item);

Path GetRangePath(uint64_t item_count, GetItem const& get_item,
                  Tree const& tree, Range const& range);

}  // namespace mkl