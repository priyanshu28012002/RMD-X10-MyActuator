#include "MyActuator.hpp"
#include "x10_api.hpp"
#include <chrono>
#include <cstring>
#include <iostream>
#include <sched.h>
#include <stdexcept>
#include <sys/mman.h>

// Static Functions
static uint64_t epochMs()
{
    using namespace std::chrono;
    return static_cast<uint64_t>(
        duration_cast<milliseconds>(system_clock::now().time_since_epoch()).count());
}

// Public Functions
MyActuator::MyActuator(int motorCount, const std::string &port)
    : motorCount_(motorCount), port_(port), driver_(std::make_unique<X10ApiSerial>(port))
{
    speeds.resize(motorCount_ + 1);
    lastSpeed.resize(motorCount_ + 1);
    setState(MyActuatorState::NONE);
    // make new thread when ever the Speed mode is set
    // commandExecuter = std::thread(&MyActuator::speedControlExecuterLoop, this);

    init();
    // int8_t result = driver_->speedControl(2, 50000);
    // std::cout << "speedControl Result " << (int)result << std::endl;

    commandExecuter = std::thread(&MyActuator::speedControlExecuterLoop, this);
    statusLoopThread = std::thread(&MyActuator::statusLoop, this);
}

MyActuator::~MyActuator()
{
    driver_->rmdX10_shut_down();
}
bool MyActuator::init()
{
    for (int i = 0; i < motorCount_ + 1; i++)
    {
        speeds[i] = 0;
    }

    driver_->rmdX10_init();
    setState(MyActuatorState::IDEAL);

    statusLoopRunningStatu = true;
    speedLoopRunningStatu = true;
    return true;
}
void MyActuator::speedControlExecuterLoop()
{
    while (speedLoopRunningStatu)
    {
        for (int i = 1; i < motorCount_ + 1; i++)
        {
            if (mode_ == MyActuatorMode::SPEED && speeds[i] != lastSpeed[i])
            {
                int8_t result = driver_->speedControl(i, speeds[i]);
                if (result == 0)
                    lastSpeed[i] = speeds[i];
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(20));
    }
    std::cout << "Speed Loop Closed" << std::endl;
}
bool MyActuator::resetMotor()
{

    for (int i = 0; i < motorCount_; i++)
    {
        int moterId = i + 1;
        driver_->Motor_reset(moterId);
    }

    std::cout << "All Motor Reset" << std::endl;
    return 0;
}
void MyActuator::setMode(MyActuatorMode newMode)
{

    mode_ = newMode;
}
MyActuatorMode MyActuator::getMode()
{
    return mode_;
}
void MyActuator::setState(MyActuatorState newState)
{

    state_ = newState;
}
MyActuatorState MyActuator::getState()
{
    return state_;
}
bool MyActuator::setMaxSpeed(int newSpeed)
{
    maxSpeed = newSpeed;
    return 0;
}
bool MyActuator::setMinSpeed(int newSpeed)
{

    minSpeed = newSpeed;
    return 0;
}
void MyActuator::setTorque(uint8_t motorId, int32_t iqControl)
{
}
void MyActuator::setSpeed(uint8_t motorId, int32_t speedDps)
{

    speeds[motorId] = speedDps;
}

bool MyActuator::setAngle(uint8_t motorId, int angle)
{
    return driver_->increment_control(motorId, maxSpeed, angle * 100);
}

void MyActuator::statusLoop()
{
    std::cout << "Status Loop Started" << std::endl;

    while (statusLoopRunningStatu)
    {

        // int state = counter % 3;
        int state = 0;

        switch (state)
        {
        case 0:
            status1();
            break;

        case 1:
            status2();
            break;

        case 2:
            status3();
            break;

        default:
            // should never happen if % 3 is used
            break;
        }
        counter++;
        std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        uint32_t runTime;

        int8_t result = driver_->Motor_runtime(1, runTime);
        std::cout << "Motor Active for " << (int)runTime / 1000<< " sec" << std::endl;
        
    }

    std::cout << "Status Loop Closed" << std::endl;
}

void MyActuator::parseMoterState3(int16_t *buffer)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);

    std::cout << "Temperature     : " << (int)data[0] << " °C" << std::endl;
    std::cout << "Phase A Current : " << (float)data[1] * 0.01 << " A" << std::endl;
    std::cout << "Phase B Current : " << (float)data[2] * 0.01 << " A" << std::endl;
    std::cout << "Phase C Current : " << (float)data[3] * 0.01 << " A" << std::endl;
    std::cout << std::endl;
}
void MyActuator::parseMoterState1(int16_t *buffer)
{
    // Each buffer element holds one byte from the DATA[] reply frame.
    // Cast to uint8_t to safely extract byte values.
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);

    int8_t temperature = (int8_t)data[1]; // 1 °C / LSB
    uint8_t brakeState = data[3];         // 1 = released, 0 = locked
    uint16_t voltage_raw = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
    float voltage = voltage_raw * 0.1f; // 0.1 V / LSB
    uint16_t errorState = (uint16_t)data[6] | ((uint16_t)data[7] << 8);

    std::cout << "Temperature     : " << (int)temperature << " °C" << std::endl;
    std::cout << "Brake State     : " << (brakeState ? "Released (1)" : "Locked (0)") << std::endl;
    std::cout << "Voltage         : " << voltage << " V" << std::endl;
    std::cout << "Error Flags     : 0x" << std::hex << errorState << std::dec << std::endl;

    // Decode individual error bits (refer to protocol section 2.14 / Failure Description)
    if (errorState == 0)
    {
        std::cout << "  No errors detected." << std::endl;
    }
    else
    {
        if (errorState & (1 << 0))
            std::cout << "  [bit0] Motor stall" << std::endl;
        if (errorState & (1 << 1))
            std::cout << "  [bit1] Low voltage" << std::endl;
        if (errorState & (1 << 2))
            std::cout << "  [bit2] Over voltage" << std::endl;
        if (errorState & (1 << 3))
            std::cout << "  [bit3] Over current" << std::endl;
        if (errorState & (1 << 4))
            std::cout << "  [bit4] Power overrun" << std::endl;
        if (errorState & (1 << 5))
            std::cout << "  [bit5] Speeding" << std::endl;
        if (errorState & (1 << 6))
            std::cout << "  [bit6] Over temperature" << std::endl;
        if (errorState & (1 << 7))
            std::cout << "  [bit7] Encoder calibration error" << std::endl;
    }
    std::cout << std::endl;
}

void MyActuator::parseMoterState2(int16_t *buffer)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);

    int8_t temperature = (int8_t)data[1];
    int16_t iq_raw = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
    float iq = iq_raw * 0.01f;                                                       // 0.01 A / LSB
    int16_t speed = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));         // 1 dps / LSB
    int16_t encoder_angle = (int16_t)((uint16_t)data[6] | ((uint16_t)data[7] << 8)); // 1 degree / LSB

    std::cout << "Temperature     : " << (int)temperature << " °C" << std::endl;
    std::cout << "Torque Current  : " << iq << " A" << std::endl;
    std::cout << "Speed           : " << (int)speed << " dps" << std::endl;
    std::cout << "Encoder Angle   : " << (int)encoder_angle << " degree" << std::endl;
    std::cout << std::endl;
}

void MyActuator::status1()
{

    int16_t buffer[16] = {0};
    int8_t x = driver_->Motor_state1(1, buffer);
    // std::cout << "Motor_state3 Result " << (int)x << std::endl;
    if (x == 0)
    {
        parseMoterState1(buffer);
    }
    std::cout << std::endl;
}

void MyActuator::status2()
{

    int16_t buffer[16] = {0};
    int8_t x = driver_->Motor_state2(1, buffer);
    // std::cout << "Motor_state3 Result " << (int)x << std::endl;
    if (x == 0)
    {
        parseMoterState2(buffer);
    }
    std::cout << std::endl;
}
void MyActuator::status3()
{

    int16_t buffer[16] = {0};
    int8_t x = driver_->Motor_state3(1, buffer);
    // std::cout << "Motor_state3 Result " << (int)x << std::endl;
    if (x == 0)
    {
        parseMoterState3(buffer);
    }
    std::cout << std::endl;
}

bool MyActuator::fini()
{
    // int8_t result = driver_->speedControl(2, 0);

    statusLoopRunningStatu = false;
    speedLoopRunningStatu = false;

    statusLoopThread.join();
    commandExecuter.join();

    return true;
}

// Private Functions
bool MyActuator::validId(uint8_t id) const
{
    return id >= 1 && id <= static_cast<uint8_t>(motorCount_);
}

uint64_t MyActuator::nowMs() const
{
    return epochMs();
}
