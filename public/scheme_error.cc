
#include "scheme_error.h"

namespace scheme {
thread_local ErrorCode last_error_code;
thread_local std::string last_error_desc;
}


