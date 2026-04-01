#pragma once

#include <thread>
#include <atomic>

#include "CommandScheduler.hpp"
#include "x10_api.hpp"
#include "MotorCommand.hpp"
#include "MotorManager.hpp"


class CommandExecuter
{
private:

    X10ApiSerial* driver_;
    CommandScheduler* scheduler_;
    MotorManager* motorManager_;

    std::thread worker_;
    std::atomic<bool> running_;

    void processLoop();
    void executeCommand(MotorCommand cmd);

public:

    CommandExecuter(X10ApiSerial* driver,
                    CommandScheduler* scheduler,
                    MotorManager *motorManager);

    ~CommandExecuter();

    void startExecution();
    void stopExecution();
    int count = 0;
};