#pragma once

#include <memory>
#include "MotorManager.hpp"
#include "CommandScheduler.hpp"
#include "CommandExecuter.hpp"

#include "x10_api.hpp"

class MyActuator
{
private:
  
    std::unique_ptr<X10ApiSerial> driver_; // low level communication
    std::unique_ptr<CommandScheduler> scheduler_; // make the motor command and set the priority of the command
    std::unique_ptr<MotorManager> motorManager_;  // manage the motor instance and status
    std::unique_ptr<CommandExecuter> executer_;  // execure the command to the motor


public:
    MyActuator(int motorCount, const std::string port);
    ~MyActuator();
    std::array<MotorState, 2> motorStatus;
    Motor *motor(int id);
};