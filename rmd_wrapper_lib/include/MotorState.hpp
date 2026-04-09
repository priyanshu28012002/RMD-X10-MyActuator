#pragma once

struct MotorState
{
    double temperature = 0;
    double voltage = 0;
    double current = 0;
    double speed = 0;
    double position = 0;
    double targetSpeed = 0;
    double torque = 0;
};