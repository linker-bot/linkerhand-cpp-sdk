#ifndef LINKERHAND_ERROR_CODE_H
#define LINKERHAND_ERROR_CODE_H

#include <system_error>
#include <stdexcept>
#include <string>

namespace linkerhand {

// LinkerHand SDK 错误码
enum class HandError {
    Success = 0,
    UnsupportedFeature,
    CommunicationError,
    InvalidParameter,
    DeviceNotFound,
    Timeout,
    InvalidState,
    OperationFailed,
    NotInitialized,
    AlreadyInitialized,
    OutOfRange,
    UnknownError
};

class HandErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "LinkerHand";
    }

    std::string message(int ev) const override {
        switch (static_cast<HandError>(ev)) {
            case HandError::Success:             return "Success";
            case HandError::UnsupportedFeature:  return "Unsupported feature";
            case HandError::CommunicationError:  return "Communication error";
            case HandError::InvalidParameter:    return "Invalid parameter";
            case HandError::DeviceNotFound:      return "Device not found";
            case HandError::Timeout:             return "Timeout";
            case HandError::InvalidState:        return "Invalid state";
            case HandError::OperationFailed:     return "Operation failed";
            case HandError::NotInitialized:      return "Not initialized";
            case HandError::AlreadyInitialized:  return "Already initialized";
            case HandError::OutOfRange:          return "Out of range";
            case HandError::UnknownError:
            default:                             return "Unknown error";
        }
    }
};

inline const HandErrorCategory& hand_error_category() {
    static HandErrorCategory instance;
    return instance;
}

inline std::error_code make_error_code(HandError e) {
    return std::error_code(static_cast<int>(e), hand_error_category());
}

class HandException : public std::runtime_error {
public:
    HandException(HandError error, const std::string& message)
        : std::runtime_error(message), error_code_(error) {}

    HandError error_code() const noexcept {
        return error_code_;
    }

private:
    HandError error_code_;
};

class UnsupportedFeatureException : public HandException {
public:
    explicit UnsupportedFeatureException(const std::string& feature_name)
        : HandException(HandError::UnsupportedFeature,
                       "Unsupported feature: " + feature_name),
          feature_name_(feature_name) {}

    const std::string& feature_name() const noexcept {
        return feature_name_;
    }

private:
    std::string feature_name_;
};

class CommunicationException : public HandException {
public:
    explicit CommunicationException(const std::string& message)
        : HandException(HandError::CommunicationError, message) {}
};

class InvalidParameterException : public HandException {
public:
    explicit InvalidParameterException(const std::string& message)
        : HandException(HandError::InvalidParameter, message) {}
};

} // namespace linkerhand

namespace std {
    template<>
    struct is_error_code_enum<linkerhand::HandError> : true_type {};
}

#endif // LINKERHAND_ERROR_CODE_H
