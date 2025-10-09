#include "openyolo/error/ErrorCodes.hpp"

namespace openyolo {
namespace error {

const char* ErrorCategory::name() const noexcept { return "openyolo"; }

std::string ErrorCategory::message(int ev) const {
    switch (static_cast<ErrorCode>(ev)) {
        case ErrorCode::Success:
            return "Success";
        case ErrorCode::InvalidArgument:
            return "Invalid argument";
        case ErrorCode::OperationNotPermitted:
            return "Operation not permitted";
        case ErrorCode::ThemeNotFound:
            return "Theme not found";
        case ErrorCode::ThemeInvalidFormat:
            return "Invalid theme format";
        case ErrorCode::ThemeInstallFailed:
            return "Failed to install theme";
        case ErrorCode::ThemeLoadFailed:
            return "Failed to load theme";
        case ErrorCode::ProfileNotFound:
            return "Profile not found";
        case ErrorCode::ProfileCorrupted:
            return "Profile data is corrupted";
        case ErrorCode::ProfileSaveFailed:
            return "Failed to save profile";
        case ErrorCode::ProfileLoadFailed:
            return "Failed to load profile";
        case ErrorCode::FileNotFound:
            return "File not found";
        case ErrorCode::PermissionDenied:
            return "Permission denied";
        case ErrorCode::OutOfMemory:
            return "Out of memory";
        case ErrorCode::SystemError:
            return "System error occurred";
        default:
            return "Unknown error";
    }
}

const ErrorCategory& ErrorCategory::instance() {
    static ErrorCategory instance;
    return instance;
}

std::error_code make_error_code(ErrorCode e) {
    return {static_cast<int>(e), ErrorCategory::instance()};
}

std::error_condition make_error_condition(ErrorCode e) {
    return {static_cast<int>(e), ErrorCategory::instance()};
}

}  // namespace error
}  // namespace openyolo
