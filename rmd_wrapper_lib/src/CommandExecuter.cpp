#include "CommandExecuter.hpp"

CommandExecuter::CommandExecuter(X10ApiSerial *driver,
                                 CommandScheduler *scheduler, MotorManager *motorManager)
    : driver_(driver),
      scheduler_(scheduler),
      motorManager_(motorManager),
      running_(false)
{
    driver_->rmdX10_init();
}

CommandExecuter::~CommandExecuter()
{
    stopExecution();
}

void CommandExecuter::startExecution()
{
    running_ = true;
    worker_ = std::thread(&CommandExecuter::processLoop, this);
}

void CommandExecuter::stopExecution()
{
    running_ = false;

    if (worker_.joinable())
        worker_.join();
}

void CommandExecuter::processLoop()
{
    while (running_)
    {
        MotorCommand cmd;

        if (!scheduler_->getNextCommand(cmd))
            continue;

        executeCommand(cmd);
    }
}

void CommandExecuter::executeCommand(MotorCommand cmd)
{

    if (cmd.type == CommandType::SPEED)
    {
        std::cout << "SPEED Command" << std::endl;

        driver_->speedControl(cmd.motorId, cmd.speed);
    }

    if (cmd.type == CommandType::POSITION)
    {
        std::cout << "POSITION Command" << std::endl;

        //  driver_->Motor_angle_control(cmd.motorId,cmd.angle,cmd.speed);
    }

    if (cmd.type == CommandType::STATUS)
    {
        count++;

        motorManager_->setAngle(cmd.motorId,count);
        motorManager_->setCurrent(cmd.motorId,count);
        motorManager_->setTemp(cmd.motorId,count);
        motorManager_->setSpeed(cmd.motorId,count);

        std::cout << "STATUS Command" << std::endl;

        // driver_->Motor_speed_control(cmd.motorId, cmd.speed);
    }

    if (cmd.type == CommandType::EMERGENCY)
    {
        std::cout << "EMERGENCY Command" << std::endl;

        //  driver_->Motor_angle_control(cmd.motorId,cmd.angle,cmd.speed);
    }
}
