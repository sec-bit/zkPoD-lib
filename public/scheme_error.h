#pragma once

#include <string>

namespace scheme {
enum ErrorCode {
  kNotDefined,
  kSuccess,
  kInternalError = -10000,
  kNotAccept,
  kInvalidFormat,
  kNotFound,
  kFileFailed,
};
extern thread_local ErrorCode last_error_code;
extern thread_local std::string last_error_desc;

inline void SetLastError(ErrorCode code = ErrorCode::kNotDefined,
                         std::string const& desc = "") {
  last_error_code = code;
  last_error_desc = desc;
}

}  // namespace scheme
