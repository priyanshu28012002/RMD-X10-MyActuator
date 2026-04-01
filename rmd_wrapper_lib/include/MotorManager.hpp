#pragma once

#include <vector>
#include <memory>
#include "Motor.hpp"

class MotorManager
{
private:
    std::vector<std::unique_ptr<Motor>> motors_;

public:
    MotorManager(int motorCount, CommandScheduler *scheduler);
    void setTemp(int id, double value);
    void setVoltage(int id, double value);
    void setCurrent(int id, double value);
    void setSpeed(int id, double value);
    void setAngle(int id, double value);

    Motor *getMotor(int id);
};