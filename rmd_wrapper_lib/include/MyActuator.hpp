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

class X10ApiSerial;


struct MotorStatus
{
    uint8_t id = 0;

    int8_t temperature = 0; 
    int16_t voltage = 0;    
    uint8_t errorState = 0; 

    int8_t temperature2 = 0;
    int16_t torqueCurrent = 0; 
    int16_t speed = 0;        
    int16_t encoder = 0;      

    int16_t phaseA = 0;
    int16_t phaseB = 0;
    int16_t phaseC = 0;

    bool online = false;
    uint64_t lastUpdated = 0;
};

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
    uint8_t motorId = 0;    
    int32_t param1 = 0;     
    int32_t param2 = 0;     
    bool emergency = false; 
};

enum class MyActuatorState
{
    IDEAL,
    READY,
    RUNNING
};

enum class MyActuatorMode
{
    NONE,
    SPEED,
    ANGLE,
    SETTING
};

class MyActuator
{
public:
    MyActuatorMode mode;

    std::vector<int> speeds;
    std::vector<int> wheelDimeter; // 160 mm

    int maxSpeed = 5000;
    int minSpeed = -5000;
    explicit MyActuator(int motorCount, const std::string &port = "/dev/ttyUSB0");
    ~MyActuator();

    MyActuator(const MyActuator &) = delete;
    MyActuator &operator=(const MyActuator &) = delete;

    void start();
    void stop();
    bool isRunning() const;
    void emergencyStopAll();
    void emergencyStop(uint8_t motorId);
    void setSpeed(uint8_t motorId, int32_t speedDps);
    void setSpeedAll(int32_t speedDps);
    void speedControlExecuterLoop();
    void setTorque(uint8_t motorId, int32_t iqControl);
    void setAbsolutePosition(uint8_t motorId, int32_t angle, uint16_t maxSpeed = 500);
    void setIncrementalPosition(uint8_t motorId, int32_t angleDelta, uint16_t maxSpeed = 500);
    void shutdownMotor(uint8_t motorId);
    void resetMotor(uint8_t motorId);
    MotorStatus getStatus(uint8_t motorId) const;
    std::vector<MotorStatus> getAllStatus() const;
    bool isMotorOnline(uint8_t motorId, uint32_t timeoutMs = 200) const;
    using StatusCallback = std::function<void(const std::vector<MotorStatus> &)>;
    void setStatusCallback(StatusCallback cb);
    using FaultCallback = std::function<void(uint8_t motorId, uint8_t errorState)>;
    void setFaultCallback(FaultCallback cb);
    uint64_t cycleCount() const;
    int64_t worstOverrunUs() const;
    bool setMaxSpeed(int newSpeed);
    bool setMinSpeed(int newSpeed);
    void setMode(MyActuatorMode newMode);
    MyActuatorMode getMode() ;
    bool checkMode(MyActuatorMode required) ;
    std::string modeToString(MyActuatorMode m);
    bool setAngle(int angle);

private:
    MyActuatorMode mode_;
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

    void rtLoop();
    void dispatchPendingCommands();
    void pollMotorStates();
    void executeCommand(const MotorCommand &cmd);
    void updateStatusTimestamp(uint8_t id);

    MotorStatus &statusRef(uint8_t id);
    const MotorStatus &statusRef(uint8_t id) const;
    bool validId(uint8_t id) const;
    uint64_t nowMs() const;
    static constexpr int LOOP_HZ = 50;
    static constexpr int LOOP_US = 1'000'000 / LOOP_HZ; // 20 000 µs
    static constexpr int MAX_CMD_PER_CYCLE = 4;         // max commands dispatched per 20 ms
    int motorCount_;
    std::string port_;
    std::unique_ptr<X10ApiSerial> driver_;
    std::thread rtThread_;
    std::thread commandExecuter;
    std::atomic<bool> running_{false};
    std::atomic<bool> stopRequested_{false};
    std::vector<MotorStatus> motorStatus_;
    mutable std::mutex statusMutex_;
    std::priority_queue<PrioritisedCommand> cmdQueue_;
    std::mutex cmdMutex_;
    StatusCallback statusCb_;
    FaultCallback faultCb_;
    mutable std::mutex cbMutex_;
    std::atomic<uint64_t> cycleCount_{0};
    std::atomic<int64_t> worstOverrunUs_{0};
    uint8_t pollRoundRobin_{0};
    int lastSpeed = 0;
};

#endif // MY_ACTUATOR_HPP