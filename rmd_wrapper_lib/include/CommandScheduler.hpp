#pragma once

#include <queue>
#include <mutex>
#include <thread>
#include "MotorCommand.hpp"
#include "x10_api.hpp"

class CommandScheduler
{
private:
    struct Compare
    {
        bool operator()(const MotorCommand &a, const MotorCommand &b)
        {
            return a.priority < b.priority;
        }
    };

    std::priority_queue<
        MotorCommand,
        std::vector<MotorCommand>,
        Compare>
        queue_;
    std::mutex mutex_;

    void processQueue();
    void execute(MotorCommand cmd);
    std::thread worker_;
    std::atomic<bool> running_;

public:
    CommandScheduler();
    void pushCommand(const MotorCommand &cmd);
    bool getNextCommand(MotorCommand &cmd);
    void startSchedulling();
    void stopSchedulling();
    void schedullingLoop();

    bool empty();
};