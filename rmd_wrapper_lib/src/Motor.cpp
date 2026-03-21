// #include "Motor.hpp"
// #include <iostream>

// Motor::Motor(int id) : id_(id)
// {
//     std::cout << "Motor Constructor id: " << id_ << std::endl;
// }

// Motor::~Motor()
// {
//     std::cout << "Motor Destructor id: " << id_ << std::endl;
// }

// // ----------- Getters -----------

// double Motor::getTemp() const
// {
//     return temp_;
// }

// double Motor::getVoltage() const
// {
//     return Voltage_;
// }

// double Motor::getCurrent() const
// {
//     return current_;
// }

// double Motor::getCurrentPose() const
// {
//     return pose_;
// }

// double Motor::getCurrentAngle() const
// {
//     return angle_;
// }

// PID Motor::getPid() const
// {
//     return pid_;
// }

// double Motor::getAccleration() const
// {

//     return acceleration_;
// }

// bool Motor::moveSpeed(double speed)
// {
//     std::cout << "Motor " << id_ 
//               << " moving at speed: " 
//               << speed << std::endl;

  
//     return true;
// }


// bool Motor::moveToAngle(double angle, double speed)
// {
//     std::cout << "Motor " << id_
//               << " moving to angle: "
//               << angle
//               << " at speed: "
//               << speed << std::endl;


//     return true;
// }

#include "Motor.hpp"
#include "CommandScheduler.hpp"

Motor::Motor(int id, CommandScheduler* scheduler)
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

// void Motor::emergency(double angle, double speed)
// {
//     MotorCommand cmd;
//     cmd.motorId = id_;
//     cmd.type = CommandType::POSITION;
//     cmd.angle = angle;
//     cmd.speed = speed;
//     cmd.priority = 1;

//     scheduler_->push(cmd);
// }