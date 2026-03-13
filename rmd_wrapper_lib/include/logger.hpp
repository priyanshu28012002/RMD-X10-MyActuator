#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include <mutex>
#include <fstream>
#include <memory>

class Logger {
public:
    enum class Level {
        DEBUG,
        INFO,
        WARNING,
        ERROR,
        CRITICAL
    };

    static Logger* get_instance();

    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void debug(const std::string& message, 
               const std::string& file = "", int line = 0);
    void info(const std::string& message, 
              const std::string& file = "", int line = 0);
    void warning(const std::string& message, 
                 const std::string& file = "", int line = 0);
    void error(const std::string& message, 
               const std::string& file = "", int line = 0);
    void critical(const std::string& message, 
                  const std::string& file = "", int line = 0);

    void set_log_level(Level level);
    void set_log_to_console(bool enable);
    void set_log_file(const std::string& filename);

private:
    Logger();
    ~Logger();

    void log(Level level, const std::string& message, 
             const std::string& file = "", int line = 0);
    
    std::string get_current_timestamp();
    std::string level_to_string(Level level);

    static Logger* instance_;
    static std::mutex mtx_;

    std::ofstream log_file_;
    Level current_level_;
    bool log_to_console_;
};

#define LOG_DEBUG(msg) Logger::get_instance()->debug(msg, __FILE__, __LINE__)
#define LOG_INFO(msg) Logger::get_instance()->info(msg, __FILE__, __LINE__)
#define LOG_WARNING(msg) Logger::get_instance()->warning(msg, __FILE__, __LINE__)
#define LOG_ERROR(msg) Logger::get_instance()->error(msg, __FILE__, __LINE__)
#define LOG_CRITICAL(msg) Logger::get_instance()->critical(msg, __FILE__, __LINE__)

#endif // LOGGER_H