#pragma once
#ifndef MY_ACTUATOR_HPP
#define MY_ACTUATOR_HPP

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <atomic>
#include <mutex>
#include <thread>
#include <condition_variable>
#include <chrono>
#include <functional>
#include <queue>
#include <array>

// Forward declare the driver
class X10ApiSerial;

// ─────────────────────────────────────────────
//  Motor State (per motor, updated every cycle)
// ─────────────────────────────────────────────
struct MotorStatus
{
    uint8_t id = 0;

    // State 1 (Motor_state1)
    int8_t temperature = 0; // °C
    int16_t voltage = 0;    // 0.1 V
    uint8_t errorState = 0; // bitmask

    // State 2 (Motor_state2)
    int8_t temperature2 = 0;
    int16_t torqueCurrent = 0; // A·0.01
    int16_t speed = 0;         // dps
    int16_t encoder = 0;       // raw

    // State 3 (Motor_state3)
    int16_t phaseA = 0;
    int16_t phaseB = 0;
    int16_t phaseC = 0;

    bool online = false;
    uint64_t lastUpdated = 0; // epoch ms
};

// ─────────────────────────────────────────────
//  Command types queued by the OS
// ─────────────────────────────────────────────
enum class CommandType : uint8_t
{
    EMERGENCY_STOP_ALL = 0,
    EMERGENCY_STOP_ONE,
    SET_SPEED,
    SET_TORQUE,
    SET_ABS_POSITION,
    SET_INC_POSITION,
    MOTOR_SHUTDOWN,
    MOTOR_RESET,
    READ_STATE1,
    READ_STATE2,
    READ_STATE3,
};

struct MotorCommand
{
    CommandType type;
    uint8_t motorId = 0;    // 0 = broadcast / all
    int32_t param1 = 0;     // speed, torque, angle, etc.
    int32_t param2 = 0;     // maxSpeed for position cmds
    bool emergency = false; // bypasses queue ordering
};

enum class MyActuatorState
{
    IDEAL,
    READY,
    RUNNING
};

// ─────────────────────────────────────────────
//  MyActuator  — Real-Time Motor OS
//
//  Single /dev/ttyUSBx channel, 50 Hz loop.
//  All motors are polled and commanded inside
//  the real-time thread; callers use the
//  thread-safe public API.
// ─────────────────────────────────────────────
class MyActuator
{
public:
    // ── Construction / Destruction ────────────
    /**
     * @param motorCount  Number of motors (IDs 1..motorCount)
     * @param port        Serial device, e.g. "/dev/ttyUSB0"
     */

    std::vector<int> speeds;
    explicit MyActuator(int motorCount, const std::string &port = "/dev/ttyUSB0");
    ~MyActuator();

    // Non-copyable, non-movable
    MyActuator(const MyActuator &) = delete;
    MyActuator &operator=(const MyActuator &) = delete;

    // ── Lifecycle ─────────────────────────────
    /** Start the 50 Hz real-time scheduler thread. */
    void start();

    /** Gracefully stop the scheduler and join thread. */
    void stop();

    /** Returns true if the RT thread is running. */
    bool isRunning() const;

    // ── Emergency ─────────────────────────────
    /**
     * Immediately stops ALL motors.
     * Posted at front of queue; executes within one cycle (<20 ms).
     */
    void emergencyStopAll();

    /**
     * Immediately stops one motor.
     */
    void emergencyStop(uint8_t motorId);

    // ── Motion Commands (queued, 50 Hz dispatch) ──
    /**
     * Set speed in dps (degrees per second).
     * Negative values reverse direction.
     */
    void setSpeed(uint8_t motorId, int32_t speedDps);

    void setSpeedAll(int32_t speedDps);
    void executerLoop();
    /**
     * Set torque current control.
     * iqControl: raw current value per X10 protocol
     */
    void setTorque(uint8_t motorId, int32_t iqControl);

    /**
     * Absolute position command.
     * @param angle    Target angle (0.01°/LSB)
     * @param maxSpeed Maximum speed limit (dps)
     */
    void setAbsolutePosition(uint8_t motorId, int32_t angle, uint16_t maxSpeed = 500);

    /**
     * Incremental position command.
     * @param angleDelta  Delta angle (0.01°/LSB)
     * @param maxSpeed    Maximum speed limit (dps)
     */
    void setIncrementalPosition(uint8_t motorId, int32_t angleDelta, uint16_t maxSpeed = 500);

    // ── Brake Control ─────────────────────────
    void brakeLock(uint8_t motorId);
    void brakeRelease(uint8_t motorId);

    // ── Motor Power ───────────────────────────
    void shutdownMotor(uint8_t motorId);
    void resetMotor(uint8_t motorId);

    // ── Status Read-back (thread-safe) ────────
    /**
     * Returns a snapshot of the last polled status for motor @p id.
     * Data is refreshed every 50 Hz cycle.
     */
    MotorStatus getStatus(uint8_t motorId) const;

    /**
     * Returns snapshots for all motors.
     */
    std::vector<MotorStatus> getAllStatus() const;

    /**
     * Returns true if the motor responded within the last @p timeoutMs ms.
     */
    bool isMotorOnline(uint8_t motorId, uint32_t timeoutMs = 200) const;

    // ── Callbacks ─────────────────────────────
    /**
     * Register a callback fired every cycle with the fresh status of each motor.
     * Called from the RT thread — keep it short!
     */
    using StatusCallback = std::function<void(const std::vector<MotorStatus> &)>;
    void setStatusCallback(StatusCallback cb);

    /**
     * Register a callback for fault events.
     * Called from the RT thread when a motor's errorState becomes non-zero.
     */
    using FaultCallback = std::function<void(uint8_t motorId, uint8_t errorState)>;
    void setFaultCallback(FaultCallback cb);

    // ── Diagnostics ───────────────────────────
    /** Returns number of RT cycles executed since start(). */
    uint64_t cycleCount() const;

    /** Returns worst-case cycle overrun in microseconds (ideal = 0). */
    int64_t worstOverrunUs() const;

private:
    // ── Internal types ────────────────────────
    struct PrioritisedCommand
    {
        bool emergency;
        MotorCommand cmd;
        bool operator<(const PrioritisedCommand &o) const
        {
            // higher priority = emergency first
            return !emergency && o.emergency;
        }
    };

    // ── RT thread entry point ─────────────────
    void rtLoop();

    // ── Per-cycle work ────────────────────────
    void dispatchPendingCommands();
    void pollMotorStates();
    void executeCommand(const MotorCommand &cmd);
    void updateStatusTimestamp(uint8_t id);

    // ── Helpers ───────────────────────────────
    MotorStatus &statusRef(uint8_t id);
    const MotorStatus &statusRef(uint8_t id) const;
    bool validId(uint8_t id) const;
    uint64_t nowMs() const;

    // ── Configuration ─────────────────────────
    static constexpr int LOOP_HZ = 50;
    static constexpr int LOOP_US = 1'000'000 / LOOP_HZ; // 20 000 µs
    static constexpr int MAX_CMD_PER_CYCLE = 4;         // max commands dispatched per 20 ms

    // ── Members ───────────────────────────────
    int motorCount_;
    std::string port_;
    std::unique_ptr<X10ApiSerial> driver_;

    // RT thread
    std::thread rtThread_;

    std::thread commandExecuter;

    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};

    // Motor state store (index 0 = motor ID 1)
    std::vector<MotorStatus> motorStatus_;
    mutable std::mutex statusMutex_;

    // Command queue (priority: emergency first)
    std::priority_queue<PrioritisedCommand> cmdQueue_;
    std::mutex cmdMutex_;

    // Callbacks
    StatusCallback statusCb_;
    FaultCallback faultCb_;
    mutable std::mutex cbMutex_;

    // Diagnostics
    std::atomic<uint64_t> cycleCount_{0};
    std::atomic<int64_t> worstOverrunUs_{0};

    // Round-robin poll index (spread state queries across cycles)
    uint8_t pollRoundRobin_{0};
};

#endif // MY_ACTUATOR_HPP