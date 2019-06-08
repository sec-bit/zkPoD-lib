#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::plain::otbatch::capi {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, std::vector<Range> const& phantoms,
          bool test_evil);
}  // namespace scheme::plain::otbatch::capi