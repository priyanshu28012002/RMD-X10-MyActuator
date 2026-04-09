#include "MotorManager.hpp"

MotorManager::MotorManager(int motorCount)
{
    for (int i = 1; i <= motorCount; i++)
    {
        motors_.push_back(std::make_unique<Motor>(i));
    }
}

void MotorManager::setTemp(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.temperature = value;
}
void MotorManager::setVoltage(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.voltage = value;
}
void MotorManager::setCurrent(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.current = value;
}
void MotorManager::setSpeed(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.speed = value;
}
void MotorManager::setPosition(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.position = value;
}
void MotorManager::setTargetSpeed(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.targetSpeed = value;
}
void MotorManager::setTorque(int id, double value)
{
    Motor *m = getMotor(id);
    m->state_.torque = value;
}

Motor *MotorManager::getMotor(int id)
{
    return motors_[id - 1].get();
}