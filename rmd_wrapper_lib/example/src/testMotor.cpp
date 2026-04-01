#include "MyActuator.hpp"
#include <iostream>
#include <thread>
#include <chrono>
#include <atomic>
#include <mutex>

constexpr int NUM_MOTORS = 1;

std::atomic<bool> running(true);

std::mutex motor_mtx;

void printStatus(MyActuator& robot)
{
    while (running.load())
    {
        {
            std::lock_guard<std::mutex> lock(motor_mtx);

            for (int i = 1; i <= NUM_MOTORS; i++)
            {
                // auto m = robot.motor(i);

                std::cout << "[Motor " << i << "] ";

                // std::cout << "Temp: "     << m->getTemp()     << " | ";
                // std::cout << "Voltage: "  << m->getVoltage()  << " | ";
                // std::cout << "Current: "  << m->getCurrent()  << " | ";
                // std::cout << "Position: " << m->getPosition() << " | ";
                // std::cout << "Angle: "    << m->getAngle();

                std::cout << std::flush;
            }

            // std::cout << "-----------------------------" << std::flush;
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main()
{
    MyActuator robot(NUM_MOTORS, "/dev/ttyUSB0");

    {
        std::lock_guard<std::mutex> lock(motor_mtx);
        // robot.motor(1)->setSpeed(200);
    }

    std::thread statusThread(printStatus, std::ref(robot));

    // Simulate main control loop
    for (int i = 0; i < 10; i++)
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    // Stop thread safely
    running.store(false);

    if (statusThread.joinable())
        statusThread.join();

    std::cout << "Program exited cleanly." << std::endl;

    return 0;
}

/*

//init
MyActuator robot(3, "/dev/ttyUSB0");
robot.motor(1)->inti();
robot.motor(2)->inti();
robot.motor(3)->inti();
robot.motor()->inti(); // For all;

//Emergency
robot.motor(1)->stop()
robot.motor(1)->applyBreak()
robot.motor(1)->EmergencyStop() // deinit,applybreak,stop

//Status
robot.motor(1)->getTemp()
robot.motor(1)->getVoltage()
robot.motor(1)->getCurrent()
robot.motor(1)->getPosition()
robot.motor(1)->getAngle()

//Command
robot.motor(2)->moveToAngle(90,100);
robot.motor(2)->moveToPosition(90,100);

//Setting
robot.motor(1)->setAcc(20)
robot.motor(1)->setPid()
robot.motor(1)->setTourqe()

*/