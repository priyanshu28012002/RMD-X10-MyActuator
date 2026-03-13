#ifndef LIBRMD_CONFIG_HPP
#define LIBRMD_CONFIG_HPP

#include <string>

enum class LogLevel
{
    TRACE,
    DEBUG,
    INFO,
    WARN,
    ERROR,
    FATAL,
    UNKNOWN
};

class Config
{
public:
    // Access singleton instance
    static Config& Instance();

    // Explicit load (call once in main or library init)
    void Load(const std::string& path);

    // Getters
    LogLevel GetLogLevel() const;
    const std::string& GetLogFile() const;

private:
    Config() = default;
    ~Config() = default;

    // Disable copy & move
    Config(const Config&) = delete;
    Config& operator=(const Config&) = delete;
    Config(Config&&) = delete;
    Config& operator=(Config&&) = delete;

private:
    LogLevel log_level_ = LogLevel::INFO;
    std::string log_file_ = "librmd.log";

    LogLevel ParseLogLevel(const std::string& level);
};

#endif