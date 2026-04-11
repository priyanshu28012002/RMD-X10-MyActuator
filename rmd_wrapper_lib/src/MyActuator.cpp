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
    commandExecuter = std::thread(&MyActuator::speedControlExecuterLoop, this);
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
    return true;
}
void MyActuator::speedControlExecuterLoop()
{
    while (true)
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
    return driver_->increment_control(motorId, maxSpeed, angle *  100);
}

bool fini()
{
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
