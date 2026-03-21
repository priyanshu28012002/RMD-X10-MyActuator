// #pragma once

// struct PID
// {
//     double pK;
//     double dK;
//     double iK;
// };

// class Motor
// {

// private:
//     int id_;
//     double temp_;
//     double Voltage_;
//     double current_;
//     double pose_;
//     double angle_;
//     double acceleration_;
//     PID pid_;

// public:
//     Motor(int id);
//     ~Motor();

//         // Getter Starus Comman

//     double getTemp() const;
//     double getVoltage() const;
//     double getCurrent() const;
//     double getCurrentPose() const;
//     double getCurrentAngle() const;
//     PID getPid() const;
//     double getAccleration() const;

//     // Setter Sertting
//     bool setPidRAM(PID pid);
//     bool setPidROM(PID pid);

//     // Control Command
//     bool moveSpeed(double speed);                 
//     bool moveToAngle(double angle, double speed); 

// };


#pragma once

#include "MotorState.hpp"
#include "MotorCommand.hpp"
#include <memory>

class CommandScheduler;

class Motor
{
private:
    int id_;
    MotorState state_;
    CommandScheduler* scheduler_;

public:
    Motor(int id, CommandScheduler* scheduler);

    int getId() const;

    MotorState getState();

    void setSpeed(double speed);
    void moveToAngle(double angle, double speed);
};