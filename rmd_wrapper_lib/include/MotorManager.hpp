#pragma once

#include <vector>
#include <memory>
#include "Motor.hpp"

class MotorManager
{
private:

    std::vector<std::unique_ptr<Motor>> motors_;

public:

    MotorManager(int motorCount, CommandScheduler* scheduler);

    Motor* getMotor(int id);
};