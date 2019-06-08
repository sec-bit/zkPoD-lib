#pragma once

#include <string>
#include "basic_types.h"

namespace scheme::plain::batch3 {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands);
}

namespace scheme::table::batch3 {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::vector<Range> const& demands);
}  // namespace scheme::table::batch3
