#include "Motor.hpp"
#include "CommandScheduler.hpp"

Motor::Motor(int id)
    : id_(id)
{
}

int Motor::getId() const
{
    return id_;
}

MotorState Motor::getState()
{
    return state_;
}

double Motor::getTemp()
{
    return state_.temperature;
}
double Motor::getVoltage()
{
    return state_.voltage;
}
double Motor::getCurrent()
{
    return state_.current;
}
double Motor::getSpeed()
{
    return state_.speed;
}

double Motor::getPosition()
{
    return state_.position;
}
double Motor::getTargetSpeed()
{
    return state_.targetSpeed;
}
double Motor::getTorque()
{
    return state_.torque;
}