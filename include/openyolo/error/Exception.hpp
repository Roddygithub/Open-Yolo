#pragma once

#include <stdexcept>
#include <string>

#include "openyolo/error/ErrorCodes.hpp"

namespace openyolo {

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& message,
                       error::ErrorCode code = static_cast<error::ErrorCode>(0));
    explicit Exception(const char* message,
                       error::ErrorCode code = static_cast<error::ErrorCode>(0));

    error::ErrorCode code() const noexcept;
    const char* what() const noexcept override;

private:
    error::ErrorCode m_code;
    std::string m_what;
};

}  // namespace openyolo
