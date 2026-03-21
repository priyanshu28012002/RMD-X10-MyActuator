#include "MyActuator.hpp"

MyActuator::MyActuator(int motorCount, const std::string port)
{
    driver_ = std::make_unique<X10ApiSerial>(port);

    scheduler_ = std::make_unique<CommandScheduler>(); // and task 

    motorManager_ = std::make_unique<MotorManager>(motorCount, scheduler_.get()); // ececute the command

    executer_ = std::make_unique<CommandExecuter>(driver_.get(), scheduler_.get());

scheduler_->startSchedulling();    
executer_->startExecution();

}

MyActuator::~MyActuator()
{
    std::cout << "MyActuator Destructor" << std::endl;
}

Motor* MyActuator::motor(int id)
{
    return motorManager_->getMotor(id);
}

