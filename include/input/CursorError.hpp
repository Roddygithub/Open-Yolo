#ifndef OPENYOLO_INPUT_CURSOR_ERROR_HPP
#define OPENYOLO_INPUT_CURSOR_ERROR_HPP

// Standard C++
#include <cerrno>
#include <cstring>
#include <stdexcept>
#include <string>

namespace openyolo {
namespace input {

// Définition de l'énumération avant toute utilisation
enum class CursorErrorCode {
    // Do not reorder these values - they are part of the public API
    // General errors
    CEC_Success = 0,
    CEC_UnknownError,

    // File errors
    CEC_FileNotFound,
    CEC_FileReadError,
    CEC_FileWriteError,
    CEC_FilePermissionDenied,
    CEC_FileAlreadyExists,

    // Profile errors
    CEC_ProfileNotFound,
    CEC_ProfileAlreadyExists,
    CEC_InvalidProfile,

    // Theme errors
    CEC_ThemeNotFound = 10,  // Explicit values to ensure stability
    CEC_InvalidTheme,
    CEC_ThemeInstallationFailed,

    // Cursor errors
    CEC_CursorNotFound,
    CEC_InvalidCursorFormat,
    CEC_CursorGenerationFailed,

    // System errors
    CEC_SystemError,
    CEC_InsufficientPermissions,
    CEC_DependencyMissing,

    // Must be the last value
    CEC_Count
};

// Déclaration de la fonction d'aide (définie après la classe)
std::string cursorErrorCodeToString(CursorErrorCode code);

// Custom exception class
class CursorError : public std::runtime_error {
public:
    // Constructor with error code and optional message
    CursorError(CursorErrorCode errorCode, const std::string& message = "");

    // Get the error code
    CursorErrorCode getCode() const { return m_code; }

    // Create an error from errno
    static CursorError fromErrno(int errnum, const std::string& path = "");

private:
    CursorErrorCode m_code;
};

// Implémentation du constructeur
inline CursorError::CursorError(CursorErrorCode errorCode, const std::string& message)
    : std::runtime_error(message.empty() ? cursorErrorCodeToString(errorCode) : message),
      m_code(errorCode) {}

// Implémentation de fromErrno
inline CursorError CursorError::fromErrno(int errnum, const std::string& path) {
    std::string message = "Error " + std::to_string(errnum) +
                          (path.empty() ? "" : " while accessing " + path) + ": " +
                          std::strerror(errnum);

    CursorErrorCode code;
    switch (errnum) {
        case ENOENT:
            code = CursorErrorCode::CEC_FileNotFound;
            break;
        case EACCES:
        case EPERM:
            code = CursorErrorCode::CEC_FilePermissionDenied;
            break;
        case EEXIST:
            code = CursorErrorCode::CEC_FileAlreadyExists;
            break;
        case ENOSPC:
        case EDQUOT:
            code = CursorErrorCode::CEC_SystemError;
            break;
        default:
            code = CursorErrorCode::CEC_SystemError;
            break;
    }

    return CursorError(code, message);
}

// Implémentation de la fonction d'aide
inline std::string cursorErrorCodeToString(CursorErrorCode code) {
    using CEC = CursorErrorCode;  // Alias pour un code plus concis
    switch (code) {
        case CEC::CEC_Success:
            return "Success";
        case CEC::CEC_UnknownError:
            return "Unknown error";
        case CEC::CEC_FileNotFound:
            return "File not found";
        case CEC::CEC_FileReadError:
            return "File read error";
        case CEC::CEC_FileWriteError:
            return "File write error";
        case CEC::CEC_FilePermissionDenied:
            return "Permission denied";
        case CEC::CEC_FileAlreadyExists:
            return "File already exists";
        case CEC::CEC_ProfileNotFound:
            return "Profile not found";
        case CEC::CEC_ProfileAlreadyExists:
            return "Profile already exists";
        case CEC::CEC_InvalidProfile:
            return "Invalid profile";
        case CEC::CEC_ThemeNotFound:
            return "Theme not found";
        case CEC::CEC_InvalidTheme:
            return "Invalid theme";
        case CEC::CEC_ThemeInstallationFailed:
            return "Theme installation failed";
        case CEC::CEC_CursorNotFound:
            return "Cursor not found";
        case CEC::CEC_InvalidCursorFormat:
            return "Invalid cursor format";
        case CEC::CEC_CursorGenerationFailed:
            return "Cursor generation failed";
        case CEC::CEC_SystemError:
            return "System error";
        case CEC::CEC_InsufficientPermissions:
            return "Insufficient permissions";
        case CEC::CEC_DependencyMissing:
            return "Missing dependency";
        case CEC::CEC_Count:  // Explicitly handle Count to avoid warning
            break;
        default:
            // This should never happen if all enum values are handled
            return "Unknown error code: " + std::to_string(static_cast<int>(code));
    }
    return "Unknown error code: " +
           std::to_string(static_cast<int>(code));  // Should not be reached
}

}  // namespace input
}  // namespace openyolo

#endif  // OPENYOLO_INPUT_CURSOR_ERROR_HPP
