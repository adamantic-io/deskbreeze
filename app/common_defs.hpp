#pragma once
#include <string>
#include <optional>

namespace dbr {

enum class ErrorCode {
    Success = 0,
    NotFound = 1,
    PermissionDenied = 2,
    InvalidInput = 3,
    Timeout = 4,
    AlreadyRunning = 5,
    NotRunning = 6,
    BindingFailed = 7,
    UnknownError = 255
};

inline bool operator! (ErrorCode c) {
    return c != ErrorCode::Success;
}

}