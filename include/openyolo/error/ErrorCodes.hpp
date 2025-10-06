#pragma once

#include <string>
#include <system_error>

namespace openyolo {
namespace error {

enum class ErrorCode {
    // Erreurs générales (0-99)
    Success = 0,
    InvalidArgument,
    OperationNotPermitted,
    
    // Erreurs de thème (100-199)
    ThemeNotFound = 100,
    ThemeInvalidFormat,
    ThemeInstallFailed,
    ThemeLoadFailed,
    
    // Erreurs de profil (200-299)
    ProfileNotFound = 200,
    ProfileCorrupted,
    ProfileSaveFailed,
    ProfileLoadFailed,
    
    // Erreurs système (300-399)
    FileNotFound = 300,
    PermissionDenied,
    OutOfMemory,
    SystemError
};

class ErrorCategory : public std::error_category {
public:
    static const ErrorCategory& instance();
    const char* name() const noexcept override;
    std::string message(int ev) const override;
    
private:
    ErrorCategory() = default;
};

std::error_code make_error_code(ErrorCode e);
std::error_condition make_error_condition(ErrorCode e);

} // namespace error
} // namespace openyolo

// Spécialisation de std::is_error_code_enum pour ErrorCode
namespace std {
    template<>
    struct is_error_code_enum<openyolo::error::ErrorCode> : public true_type {};
}
