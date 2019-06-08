#pragma once

#include <string>
#include <vector>

namespace scheme::table::otvrfq::capi {
bool Test(std::string const& publish_path, std::string const& output_path,
          std::string const& query_key,
          std::vector<std::string> const& query_values,
          std::vector<std::string> const& phantoms);
}  // namespace scheme::table::otvrfq::capi