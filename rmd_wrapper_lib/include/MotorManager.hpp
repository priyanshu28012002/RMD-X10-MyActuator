#pragma once

#include <vector>
#include <memory>
#include "Motor.hpp"

class MotorManager
{
private:
    std::vector<std::unique_ptr<Motor>> motors_;

public:
    MotorManager(int motorCount);
    void setTemp(int id, double value);
    void setVoltage(int id, double value);
    void setCurrent(int id, double value);
    void setSpeed(int id, double value);
    void setPosition(int id, double value);
    void setTargetSpeed(int id, double value);
    void setTorque(int id, double value);


    Motor *getMotor(int id);
};