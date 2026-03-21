#include "MotorManager.hpp"

MotorManager::MotorManager(int motorCount, CommandScheduler* scheduler)
{
    for(int i=1;i<=motorCount;i++)
    {
        motors_.push_back(std::make_unique<Motor>(i, scheduler));
    }
}

Motor* MotorManager::getMotor(int id)
{
    return motors_[id-1].get();
} 