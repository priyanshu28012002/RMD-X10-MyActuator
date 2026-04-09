#include "CommandScheduler.hpp"

CommandScheduler::CommandScheduler()
    : running_(true)
{
}
void CommandScheduler::pushCommand(const MotorCommand &cmd)
{
    std::lock_guard<std::mutex> lock(mutex_);
    queue_.push(cmd);
}

bool CommandScheduler::getNextCommand(MotorCommand &cmd)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (queue_.empty())
        return false;

    cmd = queue_.top();
    queue_.pop();

    return true;
}

bool CommandScheduler::empty()
{
    std::lock_guard<std::mutex> lock(mutex_);
    return queue_.empty();
}

void CommandScheduler::startStatusUpdateLoop()
{
    running_ = true;
    worker_ = std::thread(&CommandScheduler::statusUpdateLoop, this);
}

void CommandScheduler::stopSchedulling()
{
    running_ = false;

    if (worker_.joinable())
        worker_.join();
}

void CommandScheduler::statusUpdateLoop()
{
    while (running_)
    {
        MotorCommand cmd;
        cmd.motorId = 1;
        cmd.priority = 2;
        cmd.type = CommandType::STATUS;
        pushCommand(cmd);
        std::this_thread::sleep_for(std::chrono::seconds(1));
        cmd.motorId = 2;
        cmd.priority = 2;
        cmd.type = CommandType::STATUS;
        pushCommand(cmd);
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Emergency Command
// priority 5

void CommandScheduler::emergencyCommand(int id)
{
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 5;
    cmd.type = CommandType::EMERGENCY;
    pushCommand(cmd);
}

// Statue Command
// priority 3

void CommandScheduler::stateCommand(int id)
{
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 3;
    cmd.type = CommandType::STATUS;
    pushCommand(cmd);
}



// Control Command 
// priority 4
void CommandScheduler::moveAtSpeed(int id, int value)
{
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 4;
    cmd.speed = value;
    cmd.type = CommandType::SPEED;

    pushCommand(cmd);
}

void CommandScheduler::moveToPosition(int id, int value)
{
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 4;
    cmd.position = value;
    cmd.type = CommandType::POSITION;

    pushCommand(cmd);
}

// Setting Command
// priority 2
void CommandScheduler::setAcc(int id,int value){
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 2;
    cmd.angle = value;
    cmd.type = CommandType::SETTING;

    pushCommand(cmd);
}

void CommandScheduler::setPid(int id,int value){
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 2;
    cmd.angle = value;
    cmd.type = CommandType::SETTING;

    pushCommand(cmd);
}

void CommandScheduler::setTourqe(int id,int value){
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 2;
    cmd.angle = value;
    cmd.type = CommandType::SETTING;

    pushCommand(cmd);
}

void CommandScheduler::stop(int id){
    MotorCommand cmd;
    cmd.motorId = id;
    cmd.priority = 2;

    cmd.type = CommandType::STOP;

    pushCommand(cmd);
}