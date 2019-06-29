#include "c_api.h"

#include <string>

bool InitAll(std::string const& data_dir);

extern "C" {
EXPORT bool E_InitAll(char const* data_dir) {
  return InitAll(data_dir);
}
}  // extern "C"