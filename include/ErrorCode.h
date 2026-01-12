#ifndef LINKERHAND_ERROR_CODE_H
#define LINKERHAND_ERROR_CODE_H

#include <system_error>
#include <string>

namespace linkerhand {

/**
 * @brief 错误码枚举
 * 
 * 定义 LinkerHand SDK 的所有错误类型
 */
enum class HandError {
    Success = 0,                    ///< 成功
    UnsupportedFeature,            ///< 不支持的功能
    CommunicationError,            ///< 通信错误
    InvalidParameter,              ///< 无效参数
    DeviceNotFound,                ///< 设备未找到
    Timeout,                       ///< 超时
    InvalidState,                  ///< 无效状态
    OperationFailed,               ///< 操作失败
    NotInitialized,                ///< 未初始化
    AlreadyInitialized,            ///< 已初始化
    OutOfRange,                    ///< 超出范围
    UnknownError                   ///< 未知错误
};

/**
 * @brief 错误类别
 * 
 * 实现 std::error_category 接口，用于错误码分类
 */
class HandErrorCategory : public std::error_category {
public:
    const char* name() const noexcept override {
        return "LinkerHand";
    }

    std::string message(int ev) const override {
        switch (static_cast<HandError>(ev)) {
            case HandError::Success:
                return "Success";
            case HandError::UnsupportedFeature:
                return "Unsupported feature";
            case HandError::CommunicationError:
                return "Communication error";
            case HandError::InvalidParameter:
                return "Invalid parameter";
            case HandError::DeviceNotFound:
                return "Device not found";
            case HandError::Timeout:
                return "Timeout";
            case HandError::InvalidState:
                return "Invalid state";
            case HandError::OperationFailed:
                return "Operation failed";
            case HandError::NotInitialized:
                return "Not initialized";
            case HandError::AlreadyInitialized:
                return "Already initialized";
            case HandError::OutOfRange:
                return "Out of range";
            case HandError::UnknownError:
            default:
                return "Unknown error";
        }
    }
};

/**
 * @brief 获取错误类别实例
 */
inline const HandErrorCategory& hand_error_category() {
    static HandErrorCategory instance;
    return instance;
}

/**
 * @brief 将 HandError 转换为 std::error_code
 */
inline std::error_code make_error_code(HandError e) {
    return std::error_code(static_cast<int>(e), hand_error_category());
}

/**
 * @brief 异常类：LinkerHand 异常基类
 */
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

/**
 * @brief 异常类：不支持的功能
 */
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

/**
 * @brief 异常类：通信错误
 */
class CommunicationException : public HandException {
public:
    explicit CommunicationException(const std::string& message)
        : HandException(HandError::CommunicationError, message) {}
};

/**
 * @brief 异常类：无效参数
 */
class InvalidParameterException : public HandException {
public:
    explicit InvalidParameterException(const std::string& message)
        : HandException(HandError::InvalidParameter, message) {}
};

} // namespace linkerhand

// 注册错误码到标准库
namespace std {
    template<>
    struct is_error_code_enum<linkerhand::HandError> : true_type {};
}

#endif // LINKERHAND_ERROR_CODE_H
