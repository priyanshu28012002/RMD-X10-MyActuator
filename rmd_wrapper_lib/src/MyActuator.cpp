#include "MyActuator.hpp"

MyActuator::MyActuator(int motorCount, const std::string port)
{
    driver_ = std::make_unique<X10ApiSerial>(port);

    scheduler_ = std::make_unique<CommandScheduler>(); // and task

    motorManager_ = std::make_unique<MotorManager>(motorCount, scheduler_.get()); // ececute the command

    executer_ = std::make_unique<CommandExecuter>(driver_.get(), scheduler_.get(), motorManager_.get());

    scheduler_->startStatusUpdateLoop();

    executer_->startExecution();
}

MyActuator::~MyActuator()
{
    scheduler_->stopSchedulling();
    executer_->stopExecution();
    std::cout << "MyActuator Destructor" << std::endl;
}

Motor *MyActuator::getMotor(int id)
{
    return motorManager_->getMotor(id);
}

// Scheduler command

void MyActuator::emergencyCommand(int id)
{
    scheduler_->emergencyCommand(id);
}

// Statue Command
// priority 3

void MyActuator::stateCommand(int id)
{
    scheduler_->stateCommand(id);
}

// Control Command
// priority 4
void MyActuator::moveAtSpeed(int id, int value)
{
    scheduler_->moveAtSpeed(id, value);
}

void MyActuator::moveToPosition(int id, int value)
{
    scheduler_->moveToPosition(id, value);
}

// Setting Command
// priority 2
void MyActuator::setAcc(int id, int value)
{
    scheduler_->setAcc(id, value);
}

void MyActuator::setPid(int id, int value)
{
    scheduler_->setPid(id, value);
}

void MyActuator::setTourqe(int id, int value)
{
    scheduler_->setTourqe(id, value);
}

void MyActuator::stop(int id)
{
    scheduler_->stop(id);
}



