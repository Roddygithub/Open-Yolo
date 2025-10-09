#include "openyolo/error/Exception.hpp"
#include "openyolo/error/ErrorCodes.hpp"

namespace openyolo {

Exception::Exception(const std::string& message, error::ErrorCode code)
    : std::runtime_error(message)
    , m_code(code)
    , m_what(message + " (" + std::to_string(static_cast<int>(code)) + ")")
{}

Exception::Exception(const char* message, error::ErrorCode code)
    : std::runtime_error(message)
    , m_code(code)
    , m_what(std::string(message) + " (" + std::to_string(static_cast<int>(code)) + ")")
{}

error::ErrorCode Exception::code() const noexcept {
    return m_code;
}

const char* Exception::what() const noexcept {
    return m_what.c_str();
}

} // namespace openyolo
