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
    NONE,
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
    explicit MyActuator(int motorCount, const std::string &port = "/dev/ttyUSB0");
    ~MyActuator();
    // init
    bool init();
    bool resetMotor();
    // Mode
    void setMode(MyActuatorMode newMode);
    MyActuatorMode getMode();
    // State
    void setState(MyActuatorState newState);
    MyActuatorState getState();
    // Seting Mode Function
    bool setMaxSpeed(int newSpeed);
    bool setMinSpeed(int newSpeed);
    void setTorque(uint8_t motorId, int32_t iqControl);
    // Speed Mode
    void setSpeed(uint8_t motorId, int32_t speedDps);
    // Angle Mode
    bool setAngle(uint8_t motorId, int angle);
    // Finalization
    bool fini();
     void status1();
      void status2();
    void status3();
    int maxSpeed = 200;
    int minSpeed = -200;
    MyActuator(const MyActuator &) = delete;
    MyActuator &operator=(const MyActuator &) = delete;
    bool updateStatus();
    void parseMoterState1(int16_t *buffer);
    void parseMoterState2(int16_t *buffer);
    void parseMoterState3(int16_t *buffer);

    void statusLoop();

    bool statusLoopRunningStatu = false;
    bool speedLoopRunningStatu = false;
    int counter = 0;

private:
    MyActuatorMode mode_;
    MyActuatorState state_;
    std::vector<int> speeds;
    std::vector<int> lastSpeed;
    int motorCount_;
    std::string port_;
    std::unique_ptr<X10ApiSerial> driver_;
    bool validId(uint8_t id) const;
    uint64_t nowMs() const;
    void speedControlExecuterLoop();
    std::thread commandExecuter;
    std::thread statusLoopThread;
};

#endif // MY_ACTUATOR_HPP