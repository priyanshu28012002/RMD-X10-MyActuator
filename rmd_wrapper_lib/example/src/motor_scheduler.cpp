/*
 * motor_scheduler.cpp
 *
 * Priority Queue + Scheduler for MyActuator Serial Communication
 * Single file implementation — no headers
 *
 * Architecture:
 *   Application Layer (Status / Control / Emergency)
 *       └──► Priority Queue (thread-safe)
 *               └──► Scheduler (62 Hz frame loop)
 *                       └──► 10xSerial (SerialComm → SerialPort)
 *
 * Compile: g++ -std=c++17 -pthread motor_scheduler.cpp -o motor_scheduler
 */

#include <iostream>
#include <queue>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <functional>
#include <string>
#include <sstream>
#include <stdexcept>
#include <map>
#include <optional>

// ─────────────────────────────────────────────
//  SECTION 1 — ENUMS & CONSTANTS
// ─────────────────────────────────────────────

// Priority levels — lower number = higher priority
enum class Priority : int
{
    EMERGENCY = 0,   // Stop / Apply Brake       → always jumps queue
    CONTROL   = 1,   // Increment / Speed         → real-time control
    SETTINGS  = 2,   // PID / Torque / Acc        → configuration
    STATUS    = 3    // Temp / Current / Voltage   → default filler
};

// Command types mapped to your diagram
enum class CommandType
{
    // Emergency
    STOP,
    APPLY_BRAKE,

    // Control Command
    INCREMENT,
    SPEED,

    // Settings
    PID,
    TORQUE,
    ACC,

    // Status (read-only polls)
    STATUS_TEMP,
    STATUS_CURRENT,
    STATUS_VOLTAGE,
    STATUS_POSITION,
    STATUS_ANGLE
};

// Exception severity levels
enum class ExceptionLevel
{
    SOFT,   // Retry, log, continue
    HARD,   // Stop flow, corrective action
    FATAL   // Shutdown, safe state
};

// Command result returned to caller
enum class CommandResult
{
    SUCCESS,
    RETRY_EXHAUSTED,
    REJECTED_MOTOR_FAULT,
    REJECTED_QUEUE_FULL,
    REJECTED_STALE,
    PORT_DISCONNECTED,
    UNKNOWN_ERROR
};

// String helpers
static std::string priorityName(Priority p)
{
    switch (p) {
        case Priority::EMERGENCY: return "EMERGENCY";
        case Priority::CONTROL:   return "CONTROL";
        case Priority::SETTINGS:  return "SETTINGS";
        case Priority::STATUS:    return "STATUS";
    }
    return "UNKNOWN";
}

static std::string commandName(CommandType c)
{
    switch (c) {
        case CommandType::STOP:             return "STOP";
        case CommandType::APPLY_BRAKE:      return "APPLY_BRAKE";
        case CommandType::INCREMENT:        return "INCREMENT";
        case CommandType::SPEED:            return "SPEED";
        case CommandType::PID:              return "PID";
        case CommandType::TORQUE:           return "TORQUE";
        case CommandType::ACC:              return "ACC";
        case CommandType::STATUS_TEMP:      return "STATUS_TEMP";
        case CommandType::STATUS_CURRENT:   return "STATUS_CURRENT";
        case CommandType::STATUS_VOLTAGE:   return "STATUS_VOLTAGE";
        case CommandType::STATUS_POSITION:  return "STATUS_POSITION";
        case CommandType::STATUS_ANGLE:     return "STATUS_ANGLE";
    }
    return "UNKNOWN";
}

static std::string resultName(CommandResult r)
{
    switch (r) {
        case CommandResult::SUCCESS:               return "SUCCESS";
        case CommandResult::RETRY_EXHAUSTED:       return "RETRY_EXHAUSTED";
        case CommandResult::REJECTED_MOTOR_FAULT:  return "REJECTED_MOTOR_FAULT";
        case CommandResult::REJECTED_QUEUE_FULL:   return "REJECTED_QUEUE_FULL";
        case CommandResult::REJECTED_STALE:        return "REJECTED_STALE";
        case CommandResult::PORT_DISCONNECTED:     return "PORT_DISCONNECTED";
        case CommandResult::UNKNOWN_ERROR:         return "UNKNOWN_ERROR";
    }
    return "UNKNOWN";
}

// ─────────────────────────────────────────────
//  SECTION 2 — COMMAND PACKET
// ─────────────────────────────────────────────

using CommandCallback = std::function<void(CommandResult, const std::string&)>;
using Clock           = std::chrono::steady_clock;
using TimePoint       = std::chrono::time_point<Clock>;

struct Command
{
    // Identity
    uint64_t    id;            // Unique command ID (auto-assigned)
    CommandType type;
    Priority    priority;

    // Payload — generic key/value for params (e.g. {"speed": "100"})
    std::map<std::string, std::string> params;

    // Lifecycle
    TimePoint   enqueueTime;
    int         maxRetries    = 3;
    int         retryCount    = 0;
    int         ttlMs         = 500;   // Time-to-live in ms before considered stale

    // Result callback — notifies the caller asynchronously
    CommandCallback callback;

    // Comparator: lower priority value = higher urgency
    bool operator>(const Command& other) const
    {
        if (static_cast<int>(priority) != static_cast<int>(other.priority))
            return static_cast<int>(priority) > static_cast<int>(other.priority);
        // Among equal priority — older commands go first (FIFO fairness)
        return enqueueTime > other.enqueueTime;
    }
};

// ─────────────────────────────────────────────
//  SECTION 3 — EXCEPTION EVENT
// ─────────────────────────────────────────────

struct ExceptionEvent
{
    ExceptionLevel  level;
    std::string     source;    // "SERIAL", "MOTOR", "QUEUE", "WATCHDOG"
    std::string     message;
    uint64_t        commandId; // 0 if not tied to a specific command
    TimePoint       timestamp;
};

using ExceptionHandler = std::function<void(const ExceptionEvent&)>;

// ─────────────────────────────────────────────
//  SECTION 4 — MOTOR STATE (from Status polls)
// ─────────────────────────────────────────────

struct MotorState
{
    float    temperature  = 0.0f;
    float    current      = 0.0f;
    float    voltage      = 0.0f;
    float    position     = 0.0f;
    float    angle        = 0.0f;
    bool     faultActive  = false;
    TimePoint lastUpdated;
};

// ─────────────────────────────────────────────
//  SECTION 5 — MOCK SERIAL PORT
//  Replace internals with your real 10xSerial calls
// ─────────────────────────────────────────────

class SerialPort
{
public:
    explicit SerialPort(const std::string& port)
        : port_(port), connected_(false) {}

    bool connect()
    {
        // TODO: Replace with real open() call to /dev/ttyUSB0
        std::cout << "[SerialPort] Connected to " << port_ << "\n";
        connected_ = true;
        return true;
    }

    void disconnect()
    {
        connected_ = false;
        std::cout << "[SerialPort] Disconnected from " << port_ << "\n";
    }

    bool isConnected() const { return connected_; }

    // Returns true on ACK, false on timeout/error
    bool send(const Command& cmd)
    {
        if (!connected_) return false;

        // TODO: Replace with real 10xSerial frame construction + write
        // Simulate ~95% success rate for demo purposes
        bool ack = (rand() % 100) < 95;

        std::cout << "[SerialPort] TX → [" << commandName(cmd.type) << "]"
                  << " priority=" << priorityName(cmd.priority)
                  << " id=" << cmd.id
                  << " → " << (ack ? "ACK" : "NACK") << "\n";

        // Simulate serial latency
        std::this_thread::sleep_for(std::chrono::microseconds(500));

        return ack;
    }

    // Simulate reading motor status response
    MotorState readStatus()
    {
        // TODO: Replace with real frame parse from 10xSerial
        MotorState s;
        s.temperature = 35.0f + static_cast<float>(rand() % 10);
        s.current     = 1.2f  + static_cast<float>(rand() % 5) * 0.1f;
        s.voltage     = 24.0f + static_cast<float>(rand() % 3) * 0.1f;
        s.position    = static_cast<float>(rand() % 360);
        s.angle       = static_cast<float>(rand() % 360);
        s.faultActive = false;
        s.lastUpdated = Clock::now();
        return s;
    }

private:
    std::string port_;
    bool        connected_;
};

// ─────────────────────────────────────────────
//  SECTION 6 — PRIORITY QUEUE (Thread-Safe)
// ─────────────────────────────────────────────

class MotorCommandQueue
{
public:
    static constexpr int MAX_QUEUE_SIZE     = 50;   // Open slots for non-emergency
    static constexpr int EMERGENCY_SLOTS    = 2;    // Always reserved

    explicit MotorCommandQueue(ExceptionHandler exHandler)
        : exceptionHandler_(std::move(exHandler))
        , nextId_(1) {}

    // Enqueue a command — returns assigned ID or 0 on rejection
    uint64_t enqueue(CommandType    type,
                     Priority       priority,
                     std::map<std::string, std::string> params = {},
                     CommandCallback callback = nullptr,
                     int            ttlMs    = 500,
                     int            maxRetries = 3)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Check motor fault — block non-emergency commands
        if (motorFault_ && priority != Priority::EMERGENCY)
        {
            if (callback)
                callback(CommandResult::REJECTED_MOTOR_FAULT,
                         "Motor fault active — only Emergency allowed");
            return 0;
        }

        // Check capacity
        int currentSize = static_cast<int>(queue_.size());
        if (priority != Priority::EMERGENCY && currentSize >= MAX_QUEUE_SIZE)
        {
            raiseException(ExceptionLevel::SOFT, "QUEUE",
                           "Queue full — dropping incoming command", 0);
            if (callback)
                callback(CommandResult::REJECTED_QUEUE_FULL,
                         "Queue at capacity (" + std::to_string(MAX_QUEUE_SIZE) + ")");
            return 0;
        }

        Command cmd;
        cmd.id           = nextId_++;
        cmd.type         = type;
        cmd.priority     = priority;
        cmd.params       = std::move(params);
        cmd.enqueueTime  = Clock::now();
        cmd.ttlMs        = ttlMs;
        cmd.maxRetries   = maxRetries;
        cmd.retryCount   = 0;
        cmd.callback     = std::move(callback);

        queue_.push(cmd);
        cv_.notify_one();

        return cmd.id;
    }

    // Emergency shortcut — always enqueues, flushes lower priority items
    uint64_t enqueueEmergency(CommandType type,
                               CommandCallback callback = nullptr)
    {
        std::lock_guard<std::mutex> lock(mutex_);

        // Flush all non-emergency items from the queue
        std::priority_queue<Command, std::vector<Command>, std::greater<Command>> fresh;
        int flushed = 0;
        while (!queue_.empty())
        {
            auto& top = queue_.top();
            if (top.priority == Priority::EMERGENCY)
            {
                // Keep existing emergencies
                fresh.push(top);
            }
            else
            {
                // Notify callers their commands were dropped
                if (top.callback)
                    top.callback(CommandResult::REJECTED_STALE,
                                 "Flushed by emergency command");
                flushed++;
            }
            // std::priority_queue top() returns const ref, need to pop
            const_cast<std::priority_queue<Command,
                std::vector<Command>, std::greater<Command>>&>(queue_).pop();
        }
        queue_ = fresh;

        if (flushed > 0)
        {
            std::cout << "[Queue] Emergency flush — removed "
                      << flushed << " pending commands\n";
        }

        Command cmd;
        cmd.id           = nextId_++;
        cmd.type         = type;
        cmd.priority     = Priority::EMERGENCY;
        cmd.enqueueTime  = Clock::now();
        cmd.ttlMs        = 200;       // Emergency must be handled fast
        cmd.maxRetries   = 5;         // Try harder for emergency
        cmd.retryCount   = 0;
        cmd.callback     = std::move(callback);

        queue_.push(cmd);
        cv_.notify_one();

        return cmd.id;
    }

    // Blocking pop — waits until a command is available
    std::optional<Command> dequeue(int timeoutMs = 100)
    {
        std::unique_lock<std::mutex> lock(mutex_);

        bool available = cv_.wait_for(
            lock,
            std::chrono::milliseconds(timeoutMs),
            [this] { return !queue_.empty() || shutdown_; }
        );

        if (shutdown_ && queue_.empty()) return std::nullopt;
        if (!available || queue_.empty()) return std::nullopt;

        // Stale check before dequeue
        Command cmd = queue_.top();
        queue_.pop();

        auto ageMs = std::chrono::duration_cast<std::chrono::milliseconds>(
            Clock::now() - cmd.enqueueTime).count();

        if (ageMs > cmd.ttlMs && cmd.priority != Priority::EMERGENCY)
        {
            raiseException(ExceptionLevel::SOFT, "QUEUE",
                           "Stale command discarded: " + commandName(cmd.type),
                           cmd.id);
            if (cmd.callback)
                cmd.callback(CommandResult::REJECTED_STALE,
                             "Command expired after " + std::to_string(ageMs) + "ms");
            return std::nullopt;  // Caller will request next
        }

        return cmd;
    }

    void setMotorFault(bool fault)
    {
        std::lock_guard<std::mutex> lock(mutex_);
        motorFault_ = fault;
    }

    bool isMotorFault() const { return motorFault_; }

    int size() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return static_cast<int>(queue_.size());
    }

    bool empty() const
    {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    void shutdown()
    {
        std::lock_guard<std::mutex> lock(mutex_);
        shutdown_ = true;
        cv_.notify_all();
    }

private:
    void raiseException(ExceptionLevel level,
                        const std::string& source,
                        const std::string& message,
                        uint64_t cmdId)
    {
        if (exceptionHandler_)
        {
            ExceptionEvent ev;
            ev.level     = level;
            ev.source    = source;
            ev.message   = message;
            ev.commandId = cmdId;
            ev.timestamp = Clock::now();
            exceptionHandler_(ev);
        }
    }

    mutable std::mutex mutex_;
    std::condition_variable cv_;

    // Min-heap: lowest Priority int value = highest urgency
    std::priority_queue<Command,
        std::vector<Command>,
        std::greater<Command>> queue_;

    ExceptionHandler exceptionHandler_;
    std::atomic<uint64_t> nextId_;
    bool motorFault_ = false;
    bool shutdown_   = false;
};

// ─────────────────────────────────────────────
//  SECTION 7 — SCHEDULER (62 Hz Frame Loop)
// ─────────────────────────────────────────────

class MotorScheduler
{
public:
    static constexpr int    COMM_HZ               = 62;
    static constexpr int    FRAME_INTERVAL_US      = 1000000 / COMM_HZ;  // ~16129 µs
    static constexpr int    MAX_RETRIES            = 3;
    static constexpr int    WATCHDOG_TIMEOUT_MS    = 500;
    static constexpr int    SETTINGS_MAX_PER_SEC   = 5;

    MotorScheduler(std::shared_ptr<SerialPort>         serial,
                   std::shared_ptr<MotorCommandQueue>   queue,
                   ExceptionHandler                     exHandler)
        : serial_(std::move(serial))
        , queue_(std::move(queue))
        , exceptionHandler_(std::move(exHandler))
        , running_(false)
        , frameCount_(0)
        , settingsThisSecond_(0)
        , consecutiveFailures_(0)
    {}

    ~MotorScheduler() { stop(); }

    void start()
    {
        if (running_) return;
        running_ = true;

        schedulerThread_ = std::thread(&MotorScheduler::schedulerLoop, this);
        watchdogThread_  = std::thread(&MotorScheduler::watchdogLoop,  this);

        std::cout << "[Scheduler] Started at " << COMM_HZ << " Hz\n";
    }

    void stop()
    {
        if (!running_) return;
        running_ = false;
        queue_->shutdown();

        if (schedulerThread_.joinable()) schedulerThread_.join();
        if (watchdogThread_.joinable())  watchdogThread_.join();

        std::cout << "[Scheduler] Stopped. Total frames: " << frameCount_ << "\n";
    }

    MotorState getMotorState() const
    {
        std::lock_guard<std::mutex> lock(stateMutex_);
        return motorState_;
    }

    bool isRunning() const { return running_; }

private:

    // ── Main 62 Hz loop ─────────────────────────────
    void schedulerLoop()
    {
        auto nextFrame = Clock::now();

        while (running_)
        {
            auto frameStart = Clock::now();
            frameCount_++;

            // Reset per-second counters every 62 frames
            if (frameCount_ % COMM_HZ == 0)
            {
                settingsThisSecond_ = 0;
            }

            // ── Pick command for this slot ──────────────
            Command cmdToSend;
            bool    hasCommand = false;

            // Try to get from queue (non-blocking peek)
            auto maybeCmd = queue_->dequeue(0);

            if (maybeCmd.has_value())
            {
                // Settings rate limiting
                if (maybeCmd->priority == Priority::SETTINGS
                    && settingsThisSecond_ >= SETTINGS_MAX_PER_SEC)
                {
                    // Re-queue for later (push back with same data)
                    // For simplicity here we skip — in production re-enqueue it
                    raiseException(ExceptionLevel::SOFT, "SCHEDULER",
                                   "Settings rate limit hit — slot deferred",
                                   maybeCmd->id);
                }
                else
                {
                    cmdToSend  = *maybeCmd;
                    hasCommand = true;
                    if (cmdToSend.priority == Priority::SETTINGS)
                        settingsThisSecond_++;
                }
            }

            // If no command — fill slot with Status poll (never waste a cycle)
            if (!hasCommand)
            {
                cmdToSend = makeStatusPoll();
                hasCommand = true;
            }

            // ── Execute command with retry logic ────────
            if (hasCommand)
            {
                executeWithRetry(cmdToSend);
            }

            // ── Frame timing — sleep remainder of slot ──
            nextFrame += std::chrono::microseconds(FRAME_INTERVAL_US);
            auto now = Clock::now();
            if (nextFrame > now)
            {
                std::this_thread::sleep_until(nextFrame);
            }
            else
            {
                // Frame overrun — log but don't crash
                auto overrunUs = std::chrono::duration_cast<std::chrono::microseconds>(
                    now - nextFrame).count();
                if (overrunUs > 1000)  // Only warn if > 1ms overrun
                {
                    raiseException(ExceptionLevel::SOFT, "SCHEDULER",
                                   "Frame overrun by " + std::to_string(overrunUs) + "µs", 0);
                }
                nextFrame = Clock::now();
            }
        }
    }

    // ── Execute command, retry on NACK ──────────────
    void executeWithRetry(Command& cmd)
    {
        bool ack = false;

        for (int attempt = 0; attempt <= cmd.maxRetries; attempt++)
        {
            if (!serial_->isConnected())
            {
                raiseException(ExceptionLevel::HARD, "SERIAL",
                               "Port disconnected during send", cmd.id);
                attemptReconnect();
                if (cmd.callback)
                    cmd.callback(CommandResult::PORT_DISCONNECTED, "Serial port lost");
                consecutiveFailures_++;
                return;
            }

            ack = serial_->send(cmd);

            if (ack)
            {
                consecutiveFailures_ = 0;
                lastAckTime_ = Clock::now();

                // Update motor state if this was a Status command
                if (cmd.priority == Priority::STATUS)
                {
                    auto state = serial_->readStatus();
                    std::lock_guard<std::mutex> lock(stateMutex_);
                    motorState_ = state;

                    // Check motor fault from status
                    if (state.faultActive)
                    {
                        raiseException(ExceptionLevel::HARD, "MOTOR",
                                       "Motor fault flag detected in Status", cmd.id);
                        queue_->setMotorFault(true);
                    }
                }

                if (cmd.callback)
                    cmd.callback(CommandResult::SUCCESS, "ACK received");
                return;
            }

            // NACK — log and retry
            cmd.retryCount++;
            raiseException(ExceptionLevel::SOFT, "SERIAL",
                           "NACK on " + commandName(cmd.type)
                           + " attempt " + std::to_string(attempt + 1), cmd.id);

            std::this_thread::sleep_for(std::chrono::microseconds(200));
        }

        // All retries exhausted
        consecutiveFailures_++;
        raiseException(ExceptionLevel::HARD, "SERIAL",
                       "Retry exhausted for " + commandName(cmd.type), cmd.id);

        if (cmd.callback)
            cmd.callback(CommandResult::RETRY_EXHAUSTED,
                         "No ACK after " + std::to_string(cmd.maxRetries) + " retries");

        // If too many consecutive failures → escalate to fatal
        if (consecutiveFailures_ >= 5)
        {
            raiseException(ExceptionLevel::FATAL, "SCHEDULER",
                           "5 consecutive failures — triggering Emergency Stop", 0);
            triggerEmergencyStop();
        }
    }

    // ── Status poll filler command ───────────────────
    Command makeStatusPoll() const
    {
        // Rotate through all 5 status types evenly
        static const CommandType statusTypes[] = {
            CommandType::STATUS_TEMP,
            CommandType::STATUS_CURRENT,
            CommandType::STATUS_VOLTAGE,
            CommandType::STATUS_POSITION,
            CommandType::STATUS_ANGLE
        };
        static int index = 0;

        Command cmd;
        cmd.id          = 0;  // Internal filler — no ID needed
        cmd.type        = statusTypes[index % 5];
        cmd.priority    = Priority::STATUS;
        cmd.enqueueTime = Clock::now();
        cmd.ttlMs       = 1000;
        cmd.maxRetries  = 1;   // Status polls don't need aggressive retry

        index++;
        return cmd;
    }

    // ── Watchdog loop (independent thread) ──────────
    void watchdogLoop()
    {
        while (running_)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            auto now       = Clock::now();
            auto sinceAck  = std::chrono::duration_cast<std::chrono::milliseconds>(
                                 now - lastAckTime_).count();

            if (sinceAck > WATCHDOG_TIMEOUT_MS && frameCount_ > COMM_HZ)
            {
                raiseException(ExceptionLevel::FATAL, "WATCHDOG",
                               "No ACK for " + std::to_string(sinceAck) + "ms — Emergency Stop",
                               0);
                triggerEmergencyStop();
            }
        }
    }

    // ── Auto reconnect attempt ───────────────────────
    void attemptReconnect()
    {
        std::cout << "[Scheduler] Attempting reconnect...\n";
        serial_->disconnect();
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        if (!serial_->connect())
        {
            raiseException(ExceptionLevel::FATAL, "SERIAL",
                           "Reconnect failed", 0);
        }
    }

    // ── Internal Emergency Stop ──────────────────────
    void triggerEmergencyStop()
    {
        std::cout << "[Scheduler] !! AUTO EMERGENCY STOP TRIGGERED !!\n";
        queue_->enqueueEmergency(CommandType::STOP);
    }

    // ── Exception relay ─────────────────────────────
    void raiseException(ExceptionLevel level,
                        const std::string& source,
                        const std::string& message,
                        uint64_t cmdId)
    {
        if (exceptionHandler_)
        {
            ExceptionEvent ev;
            ev.level     = level;
            ev.source    = source;
            ev.message   = message;
            ev.commandId = cmdId;
            ev.timestamp = Clock::now();
            exceptionHandler_(ev);
        }
    }

    // ── Members ─────────────────────────────────────
    std::shared_ptr<SerialPort>        serial_;
    std::shared_ptr<MotorCommandQueue> queue_;
    ExceptionHandler                   exceptionHandler_;

    std::atomic<bool>    running_;
    std::thread          schedulerThread_;
    std::thread          watchdogThread_;

    std::atomic<int>     frameCount_;
    std::atomic<int>     settingsThisSecond_;
    std::atomic<int>     consecutiveFailures_;

    TimePoint            lastAckTime_    = Clock::now();

    mutable std::mutex   stateMutex_;
    MotorState           motorState_;
};

// ─────────────────────────────────────────────
//  SECTION 8 — EXCEPTION BUS (Application Layer)
// ─────────────────────────────────────────────

class ExceptionBus
{
public:
    ExceptionBus()
    {
        // Default handler — prints to console
        // In your real app: route to logger, alert system, or ROS topic
        handler_ = [](const ExceptionEvent& ev)
        {
            const char* levelStr =
                ev.level == ExceptionLevel::SOFT  ? "[ SOFT ]" :
                ev.level == ExceptionLevel::HARD  ? "[ HARD ]" : "[FATAL ]";

            std::cout << "[ExceptionBus] " << levelStr
                      << " [" << ev.source << "] "
                      << ev.message;
            if (ev.commandId > 0)
                std::cout << " (cmd#" << ev.commandId << ")";
            std::cout << "\n";
        };
    }

    ExceptionHandler getHandler() { return handler_; }

    void setCustomHandler(ExceptionHandler h) { handler_ = std::move(h); }

private:
    ExceptionHandler handler_;
};

// ─────────────────────────────────────────────
//  SECTION 9 — APPLICATION FACADE (MyActuator)
//  This is what your app code calls
// ─────────────────────────────────────────────

class MyActuator
{
public:
    explicit MyActuator(const std::string& port = "/dev/ttyUSB0")
    {
        exBus_    = std::make_shared<ExceptionBus>();
        serial_   = std::make_shared<SerialPort>(port);
        queue_    = std::make_shared<MotorCommandQueue>(exBus_->getHandler());
        scheduler_= std::make_shared<MotorScheduler>(serial_, queue_, exBus_->getHandler());
    }

    bool connect()
    {
        if (!serial_->connect()) return false;
        scheduler_->start();
        return true;
    }

    void disconnect()
    {
        scheduler_->stop();
        serial_->disconnect();
    }

    // ── Emergency ───────────────────────────────────
    uint64_t stop(CommandCallback cb = nullptr)
    {
        std::cout << "[MyActuator] STOP requested\n";
        return queue_->enqueueEmergency(CommandType::STOP, std::move(cb));
    }

    uint64_t applyBrake(CommandCallback cb = nullptr)
    {
        std::cout << "[MyActuator] APPLY_BRAKE requested\n";
        return queue_->enqueueEmergency(CommandType::APPLY_BRAKE, std::move(cb));
    }

    // ── Control Commands ────────────────────────────
    uint64_t setSpeed(float value, CommandCallback cb = nullptr)
    {
        return queue_->enqueue(CommandType::SPEED, Priority::CONTROL,
                               {{"value", std::to_string(value)}},
                               std::move(cb));
    }

    uint64_t setIncrement(float value, CommandCallback cb = nullptr)
    {
        return queue_->enqueue(CommandType::INCREMENT, Priority::CONTROL,
                               {{"value", std::to_string(value)}},
                               std::move(cb));
    }

    // ── Settings ────────────────────────────────────
    uint64_t setPID(float p, float i, float d, CommandCallback cb = nullptr)
    {
        return queue_->enqueue(CommandType::PID, Priority::SETTINGS,
                               {{"p", std::to_string(p)},
                                {"i", std::to_string(i)},
                                {"d", std::to_string(d)}},
                               std::move(cb));
    }

    uint64_t setTorque(float value, CommandCallback cb = nullptr)
    {
        return queue_->enqueue(CommandType::TORQUE, Priority::SETTINGS,
                               {{"value", std::to_string(value)}},
                               std::move(cb));
    }

    uint64_t setAcc(float value, CommandCallback cb = nullptr)
    {
        return queue_->enqueue(CommandType::ACC, Priority::SETTINGS,
                               {{"value", std::to_string(value)}},
                               std::move(cb));
    }

    // ── State ────────────────────────────────────────
    MotorState getState() const { return scheduler_->getMotorState(); }
    int        queueSize() const { return queue_->size(); }
    bool       isConnected() const { return serial_->isConnected(); }

private:
    std::shared_ptr<ExceptionBus>        exBus_;
    std::shared_ptr<SerialPort>          serial_;
    std::shared_ptr<MotorCommandQueue>   queue_;
    std::shared_ptr<MotorScheduler>      scheduler_;
};


int main()
{
    std::cout << "=== MyActuator Scheduler Demo ===\n\n";

    MyActuator motor("/dev/ttyUSB0");

    if (!motor.connect())
    {
        std::cerr << "Failed to connect\n";
        return 1;
    }

    // Small delay to let scheduler warm up
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // ── Send a burst of mixed-priority commands ──────
    std::cout << "\n--- Sending mixed commands ---\n";

    motor.setPID(1.2f, 0.5f, 0.1f, [](CommandResult r, const std::string& msg)
    {
        std::cout << "[CB] PID → " << resultName(r) << " | " << msg << "\n";
    });

    motor.setSpeed(100.0f, [](CommandResult r, const std::string& msg)
    {
        std::cout << "[CB] Speed → " << resultName(r) << " | " << msg << "\n";
    });

    motor.setIncrement(10.0f, [](CommandResult r, const std::string& msg)
    {
        std::cout << "[CB] Increment → " << resultName(r) << " | " << msg << "\n";
    });

    motor.setTorque(0.8f, [](CommandResult r, const std::string& msg)
    {
        std::cout << "[CB] Torque → " << resultName(r) << " | " << msg << "\n";
    });

    // Let scheduler drain the queue
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // ── Simulate Emergency mid-flight ───────────────
    std::cout << "\n--- Triggering Emergency STOP ---\n";

    motor.setSpeed(200.0f);   // Will be flushed by emergency
    motor.setSpeed(300.0f);   // Will be flushed by emergency

    motor.stop([](CommandResult r, const std::string& msg)
    {
        std::cout << "[CB] Emergency STOP → " << resultName(r) << " | " << msg << "\n";
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // ── Print final motor state ─────────────────────
    auto state = motor.getState();
    std::cout << "\n--- Motor State ---\n";
    std::cout << "  Temp:     " << state.temperature << " °C\n";
    std::cout << "  Current:  " << state.current     << " A\n";
    std::cout << "  Voltage:  " << state.voltage     << " V\n";
    std::cout << "  Position: " << state.position    << " \n";
    std::cout << "  Angle:    " << state.angle       << " °\n";
    std::cout << "  Fault:    " << (state.faultActive ? "YES" : "NO") << "\n";

    std::cout << "\n--- Queue remaining: " << motor.queueSize() << " ---\n";

    motor.disconnect();

    std::cout << "\n=== Done ===\n";
    return 0;
}
