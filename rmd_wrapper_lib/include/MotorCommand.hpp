#pragma once

enum class CommandType
{
    SPEED,
    POSITION,
    STATUS,
    EMERGENCY,
    SETTING
};

struct MotorCommand
{
    int motorId;
    CommandType type;

    double speed = 0;
    double angle = 0;
    int priority = 0;
    int position = 0;
};