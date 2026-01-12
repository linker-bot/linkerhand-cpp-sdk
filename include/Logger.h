#ifndef LINKERHAND_LOGGER_H
#define LINKERHAND_LOGGER_H

#include <iostream>
#include <string>
#include <sstream>
#include <mutex>
#include <memory>

namespace linkerhand {
namespace logging {

/**
 * @brief 日志级别枚举
 */
enum class LogLevel {
    TRACE = 0,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    CRITICAL,
    OFF
};

/**
 * @brief 日志接口
 * 
 * 定义日志系统的接口，可以替换为 spdlog 或其他日志库
 */
class ILogger {
public:
    virtual ~ILogger() = default;
    
    virtual void log(LogLevel level, const std::string& message) = 0;
    virtual void setLevel(LogLevel level) = 0;
    virtual LogLevel getLevel() const = 0;
    
    void trace(const std::string& message) { log(LogLevel::TRACE, message); }
    void debug(const std::string& message) { log(LogLevel::DEBUG, message); }
    void info(const std::string& message) { log(LogLevel::INFO, message); }
    void warn(const std::string& message) { log(LogLevel::WARN, message); }
    void error(const std::string& message) { log(LogLevel::ERROR, message); }
    void critical(const std::string& message) { log(LogLevel::CRITICAL, message); }
};

/**
 * @brief 简单控制台日志实现
 * 
 * 使用 std::cout 和 std::cerr 输出日志
 * 可以后续替换为 spdlog
 */
class ConsoleLogger : public ILogger {
public:
    explicit ConsoleLogger(LogLevel level = LogLevel::INFO) 
        : currentLevel_(level) {}
    
    void log(LogLevel level, const std::string& message) override {
        if (level < currentLevel_ || level == LogLevel::OFF) {
            return;
        }
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        const char* levelStr = levelToString(level);
        std::ostream& stream = (level >= LogLevel::ERROR) ? std::cerr : std::cout;
        
        stream << "[" << levelStr << "] " << message << std::endl;
    }
    
    void setLevel(LogLevel level) override {
        std::lock_guard<std::mutex> lock(mutex_);
        currentLevel_ = level;
    }
    
    LogLevel getLevel() const override {
        return currentLevel_;
    }

private:
    static const char* levelToString(LogLevel level) {
        switch (level) {
            case LogLevel::TRACE: return "TRACE";
            case LogLevel::DEBUG: return "DEBUG";
            case LogLevel::INFO: return "INFO";
            case LogLevel::WARN: return "WARN";
            case LogLevel::ERROR: return "ERROR";
            case LogLevel::CRITICAL: return "CRITICAL";
            default: return "UNKNOWN";
        }
    }
    
    LogLevel currentLevel_;
    mutable std::mutex mutex_;
};

/**
 * @brief 空日志实现（禁用日志）
 */
class NullLogger : public ILogger {
public:
    void log(LogLevel, const std::string&) override {}
    void setLevel(LogLevel) override {}
    LogLevel getLevel() const override { return LogLevel::OFF; }
};

/**
 * @brief 全局日志管理器
 */
class Logger {
public:
    /**
     * @brief 获取默认日志实例
     */
    static ILogger& get() {
        static std::unique_ptr<ILogger> instance = 
            std::make_unique<ConsoleLogger>(LogLevel::INFO);
        return *instance;
    }
    
    /**
     * @brief 设置日志实例
     */
    static void setLogger(std::unique_ptr<ILogger> logger) {
        static std::unique_ptr<ILogger> instance = std::move(logger);
        get() = *instance;
    }
    
    /**
     * @brief 便捷方法：记录 TRACE 日志
     */
    static void trace(const std::string& message) {
        get().trace(message);
    }
    
    /**
     * @brief 便捷方法：记录 DEBUG 日志
     */
    static void debug(const std::string& message) {
        get().debug(message);
    }
    
    /**
     * @brief 便捷方法：记录 INFO 日志
     */
    static void info(const std::string& message) {
        get().info(message);
    }
    
    /**
     * @brief 便捷方法：记录 WARN 日志
     */
    static void warn(const std::string& message) {
        get().warn(message);
    }
    
    /**
     * @brief 便捷方法：记录 ERROR 日志
     */
    static void error(const std::string& message) {
        get().error(message);
    }
    
    /**
     * @brief 便捷方法：记录 CRITICAL 日志
     */
    static void critical(const std::string& message) {
        get().critical(message);
    }
    
    /**
     * @brief 设置日志级别
     */
    static void setLevel(LogLevel level) {
        get().setLevel(level);
    }
    
    /**
     * @brief 获取日志级别
     */
    static LogLevel getLevel() {
        return get().getLevel();
    }
};

} // namespace logging
} // namespace linkerhand

// 便捷宏定义
#define LINKERHAND_LOG_TRACE(msg) linkerhand::logging::Logger::trace(msg)
#define LINKERHAND_LOG_DEBUG(msg) linkerhand::logging::Logger::debug(msg)
#define LINKERHAND_LOG_INFO(msg) linkerhand::logging::Logger::info(msg)
#define LINKERHAND_LOG_WARN(msg) linkerhand::logging::Logger::warn(msg)
#define LINKERHAND_LOG_ERROR(msg) linkerhand::logging::Logger::error(msg)
#define LINKERHAND_LOG_CRITICAL(msg) linkerhand::logging::Logger::critical(msg)

#endif // LINKERHAND_LOGGER_H
