#include "joystick.cpp"
#include "MyActuator.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>

constexpr int NUM_MOTORS = 2; // Motor id 1 and 2 1 for the left and 2 right
constexpr const char *MOTOR_PORT = "/dev/ttyUSB0";
constexpr const char *JOYSTICK_DEV = "/dev/input/js0";

static std::atomic<bool> g_shutdown{false};

static void sigHandler(int)
{
    g_shutdown.store(true);
}

static bool risingEdge(bool current, bool &prev)
{
    bool edge = current && !prev;
    prev = current;
    return edge;
}

int main()
{
    std::signal(SIGINT, sigHandler);
    std::signal(SIGTERM, sigHandler);

    Joystick joy(JOYSTICK_DEV);
    if (!joy.isConnected())
    {
        std::cerr << "[FATAL] Cannot open joystick " << JOYSTICK_DEV << "\n";
        return 1;
    }

    MyActuator actuator(NUM_MOTORS, MOTOR_PORT);
actuator.init();


    bool speedMode = true;
    bool prevBtn[12] = {};

    using Clock = std::chrono::steady_clock;
    auto nextWake = Clock::now();
    bool isJoystickActive = false;
    bool isMotorInit = false;

    float targetAngle = 0.0f;
    const float ANGLE_INCREMENT = 360.0f;

    int count = 0;
    while (!g_shutdown.load())
    {
        joy.poll();
        const JoyCommand &js = joy.getState();

        bool cur[12];
        for (int i = 0; i < 12; ++i)
            cur[i] = js.button[i];

        if (risingEdge(cur[4], prevBtn[4]))
        {
            std::string status = "Inactive";
            isJoystickActive = !isJoystickActive;
            if (isJoystickActive == 0)
            {
                status = "Inactive";
            }
            else
            {
                status = "Active";
            }
            std::cout << "[INFO] Joystick " << status << std::endl;
        }

        if (risingEdge(cur[5], prevBtn[5]))
        {
            g_shutdown.store(true);
            std::cout << "[INFO] Stopping motors and exiting...\n";
            exit(0);
        }

        if (risingEdge(cur[1], prevBtn[1]))
        {
            actuator.setMode(MyActuatorMode::SPEED);
            speedMode = true;
            std::cout << "[INFO] Switched to SPEED mode\n";
        }

        if (risingEdge(cur[0], prevBtn[0]) && isJoystickActive)
        {
            actuator.setMode(MyActuatorMode::ANGLE);
            speedMode = false;

            targetAngle = ANGLE_INCREMENT;


            actuator.setAngle(2,360*4);
            std::cout << "[INFO] Switched to ANGLE mode, target angle: " << targetAngle << " degrees\n";
        }

        if (risingEdge(cur[2], prevBtn[2]) && isJoystickActive) 
        {
            actuator.resetMotor();
        }

        if (isJoystickActive)
        {
            if (speedMode)
            {
                float speed = js.axis[3];
                actuator.setSpeed(2,speed);

                // if (speed == 0)
                // {
                //     count++;
                //     std::cout << "[SPEED] Axis 3 value: " << count << "\n";
                // }
            }
            else
            {
            }
        }

        for (int i = 0; i < 12; ++i)
            prevBtn[i] = cur[i];

        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

// if (risingEdge(cur[0], prevBtn[0]))
// {
//     isMotorInit = !isMotorInit;

//     std::cout << "Init " << isMotorInit << "\n";
// }actuator

// if (risingEdge(cur[2], prevBtn[2]))
// {
//     std::cout << "[NULL] \n";
// }

// if (risingEdge(cur[3], prevBtn[3]))
// {
//     std::cout << "[NULL] \n";
// }

// if (risingEdge(cur[4], prevBtn[4]))
// {
//     std::cout << "[NULL] \n";
// }