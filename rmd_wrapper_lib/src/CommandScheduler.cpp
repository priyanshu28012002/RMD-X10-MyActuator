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

void CommandScheduler::startSchedulling()
{
    running_ = true;
    worker_ = std::thread(&CommandScheduler::schedullingLoop, this);
}

void CommandScheduler::stopSchedulling()
{
     running_ = false;

    if (worker_.joinable())
        worker_.join();
}

void CommandScheduler::schedullingLoop()
{
     while (running_)
    {
        MotorCommand cmd;
        cmd.motorId =1;
        cmd.priority = 2;
        cmd.type = CommandType::EMERGENCY;

        pushCommand(cmd);
        
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

// Statue Command

// Control COmmand

// Setting Command
