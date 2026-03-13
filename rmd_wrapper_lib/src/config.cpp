#include "config.hpp"

#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>

static std::string trim(const std::string& str)
{
    const char* ws = " \t\n\r";
    size_t start = str.find_first_not_of(ws);
    size_t end = str.find_last_not_of(ws);

    if (start == std::string::npos)
        return "";

    return str.substr(start, end - start + 1);
}

static std::string toUpper(std::string s)
{
    std::transform(s.begin(), s.end(), s.begin(),
                   [](unsigned char c){ return std::toupper(c); });
    return s;
}

Config& Config::Instance()
{
    static Config instance;  // Meyers Singleton
    return instance;
}

LogLevel Config::ParseLogLevel(const std::string& level)
{
    std::string val = toUpper(level);

    if (val == "TRACE") return LogLevel::TRACE;
    if (val == "DEBUG") return LogLevel::DEBUG;
    if (val == "INFO")  return LogLevel::INFO;
    if (val == "WARN")  return LogLevel::WARN;
    if (val == "ERROR") return LogLevel::ERROR;
    if (val == "FATAL") return LogLevel::FATAL;

    return LogLevel::UNKNOWN;
}

void Config::Load(const std::string& path)
{
    std::ifstream file(path);
    if (!file.is_open())
        throw std::runtime_error("Unable to open config file: " + path);

    std::string line;

    while (std::getline(file, line))
    {
        line = trim(line);

        if (line.empty() || line[0] == '#')
            continue;

        std::stringstream ss(line);
        std::string key, value;

        if (std::getline(ss, key, '=') &&
            std::getline(ss, value))
        {
            key = trim(key);
            value = trim(value);

            if (key == "log_level")
                log_level_ = ParseLogLevel(value);

            else if (key == "log_file")
                log_file_ = value;
        }
    }
}

LogLevel Config::GetLogLevel() const
{
    return log_level_;
}

const std::string& Config::GetLogFile() const
{
    return log_file_;
}