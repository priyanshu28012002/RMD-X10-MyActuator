#include "Motor.hpp"
#include "CommandScheduler.hpp"

Motor::Motor(int id, CommandScheduler *scheduler)
    : id_(id), scheduler_(scheduler)
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


void Motor::setSpeed(double speed)
{
    MotorCommand cmd;
    cmd.motorId = id_;
    cmd.type = CommandType::SPEED;
    cmd.speed = speed;
    cmd.priority = 2;

    scheduler_->pushCommand(cmd);
}

void Motor::moveToAngle(double angle, double speed)
{
    MotorCommand cmd;
    cmd.motorId = id_;
    cmd.type = CommandType::POSITION;
    cmd.angle = angle;
    cmd.speed = speed;
    cmd.priority = 1;

    scheduler_->pushCommand(cmd);
}

// void Motor::updateState()
// {
//     state_.temperature++;
//     state_.voltage++;
//     state_.current++;
//     state_.angle++;
//     state_.speed++;
// }