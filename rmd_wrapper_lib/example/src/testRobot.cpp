#include "joystick.cpp"
#include "MyActuator.hpp"

#include <atomic>
#include <chrono>
#include <csignal>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <thread>

constexpr int NUM_MOTORS = 2;
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

    bool speedMode = true;
    bool prevBtn[12] = {};

    using Clock = std::chrono::steady_clock;
    auto nextWake = Clock::now();
    bool isJoystickActive = false;
    bool isMotorInit = false;

    // Angle mode variables
    float targetAngle = 0.0f;
    const float ANGLE_INCREMENT = 360.0f; // Increment by 360 degrees each press

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

        // Button 5: Stop and exit
        if (risingEdge(cur[5], prevBtn[5]))
        {
            actuator.stop();
            g_shutdown.store(true);
            std::cout << "[INFO] Stopping motors and exiting...\n";
            exit(0);
        }

        // Button 1: Speed mode
        if (risingEdge(cur[1], prevBtn[1]))
        {
            actuator.setMode(MyActuatorMode::SPEED);
            speedMode = true;
            std::cout << "[INFO] Switched to SPEED mode\n";
        }

        // Button 0: Angle mode with angle increment
        if (risingEdge(cur[0], prevBtn[0]) && isJoystickActive)
        {
            actuator.setMode(MyActuatorMode::ANGLE);
            speedMode = false;

            // Increment target angle by 360 degrees
            targetAngle = ANGLE_INCREMENT;

            // Optional: Keep angle within 0-360 range or allow continuous
            // targetAngle = fmod(targetAngle, 360.0f);

            actuator.setAngle(targetAngle);
            std::cout << "[INFO] Switched to ANGLE mode, target angle: " << targetAngle << " degrees\n";
        }

        // Handle joystick control based on current mode
        if (isJoystickActive)
        {
            if (speedMode)
            {
                float speed = js.axis[3];
                actuator.setSpeedAll(speed);
                // std::cout << "[SPEED] Axis 3 value: " << speed << "\n";
            }
            else
            {
                
            }
        }

        // Copy current button states to prev for next iteration
        for (int i = 0; i < 12; ++i)
            prevBtn[i] = cur[i];

        // Add small delay to prevent excessive CPU usage
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }

    return 0;
}

// if (risingEdge(cur[0], prevBtn[0]))
// {
//     isMotorInit = !isMotorInit;

//     std::cout << "Init " << isMotorInit << "\n";
// }

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