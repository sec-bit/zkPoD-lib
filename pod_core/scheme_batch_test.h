#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::plain::batch {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil);
} // namespace scheme::plain::batch

namespace scheme::table::batch {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands, bool test_evil);
}  // namespace scheme::table::batch
