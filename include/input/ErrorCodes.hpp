#pragma once

#include <string>
#include <stdexcept>

namespace input {

enum class ErrorCode {
    Success = 0,
    DBusConnectionFailed,
    DBusProxyCreationFailed,
    SessionCreationFailed,
    InvalidAccelerator,
    ShortcutRegistrationFailed,
    NotInitialized,
    InvalidState,
    FileOperationFailed,
    InvalidParameter
};

class InputBackendError : public std::runtime_error {
public:
    explicit InputBackendError(ErrorCode code, const std::string& message)
        : std::runtime_error(message), m_code(code) {}
    
    ErrorCode code() const { return m_code; }
    
    const char* codeAsString() const {
        switch (m_code) {
            case ErrorCode::Success: return "Success";
            case ErrorCode::DBusConnectionFailed: return "DBusConnectionFailed";
            case ErrorCode::DBusProxyCreationFailed: return "DBusProxyCreationFailed";
            case ErrorCode::SessionCreationFailed: return "SessionCreationFailed";
            case ErrorCode::InvalidAccelerator: return "InvalidAccelerator";
            case ErrorCode::ShortcutRegistrationFailed: return "ShortcutRegistrationFailed";
            case ErrorCode::NotInitialized: return "NotInitialized";
            case ErrorCode::InvalidState: return "InvalidState";
            case ErrorCode::FileOperationFailed: return "FileOperationFailed";
            case ErrorCode::InvalidParameter: return "InvalidParameter";
            default: return "Unknown";
        }
    }

private:
    ErrorCode m_code;
};

} // namespace input
