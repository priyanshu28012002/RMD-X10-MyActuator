#pragma once

#include <memory>
#include "MotorManager.hpp"
#include "CommandScheduler.hpp"
#include "CommandExecuter.hpp"

#include "x10_api.hpp"

class MyActuator
{
private:
    std::unique_ptr<X10ApiSerial> driver_;        // low level communication
    std::unique_ptr<CommandScheduler> scheduler_; // make the motor command and set the priority of the command
    std::unique_ptr<MotorManager> motorManager_;  // manage the motor instance and status
    std::unique_ptr<CommandExecuter> executer_;   // execure the command to the motor

public:
    MyActuator(int motorCount, const std::string port);
    ~MyActuator();

    Motor *getMotor(int id);


        void emergencyCommand(int id);

    void stateCommand(int id);
    void moveAtSpeed(int id,int value);
    void moveToPosition(int id,int value);
    void setAcc(int id,int value);
    void stop(int id,int value);
    void setPid(int id,int value);
    void setTourqe(int id,int value);


    
};