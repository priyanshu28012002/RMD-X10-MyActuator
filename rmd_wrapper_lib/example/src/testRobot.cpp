// #include "joystick.cpp"
// #include "MyActuator.hpp"

// constexpr int NUM_MOTORS = 2;

// bool robotRunningState = true;

// void printStatus(MyActuator &robot)
// {
//     while (robotRunningState)
//     {
//         std::cout << "\r"; // move cursor to start

//         for (int i = 1; i <= NUM_MOTORS; i++)
//         {
//             Motor *m = robot.getMotor(i);
//             if (!m)
//                 continue;

//             std::cout << "[M" << i << "] ";

//             std::cout << "T:" << m->getTemp() << " ";
//             std::cout << "V:" << m->getVoltage() << " ";
//             std::cout << "I:" << m->getCurrent() << " ";
//             std::cout << "S:" << m->getSpeed() << " ";
//             std::cout << "P:" << m->getPosition() << " ";
//             std::cout << "TS:" << m->getTargetSpeed() << " ";
//             std::cout << "TQ:" << m->getTorque() << " | ";
//         }

//         std::cout << std::flush;
//         std::this_thread::sleep_for(std::chrono::milliseconds(100));
//     }
// }

// JoyCommand state;
// JoyCommand rawState;
// int inputMax = 32767;

// int inputMin = -32767;

// int outputMax = 50;

// int outputMin = -50;

// void printState(JoyCommand state)
// {
//     std::cout << "Buttons: ";
//     for (int i = 0; i < 12; i++)
//         std::cout << state.button[i] << " ";

//     std::cout << "\nAxes: ";
//     for (int i = 0; i < 6; i++)
//         std::cout << state.axis[i] << " ";

//     std::cout << "\n---------------------\n";
// }

// JoyCommand normalizeJoyState(const JoyCommand &raw)
// {
//     JoyCommand normalized = raw;

//     const int inputMax = 32767;
//     const int inputMin = -32767;
//     const int outputMax = 5;
//     const int outputMin = -5;

//     for (int i = 0; i < 6; i++)
//     {
//         int x = raw.axis[i];

//         // Clamp input (safety)
//         if (x > inputMax)
//             x = inputMax;
//         if (x < inputMin)
//             x = inputMin;

//         float scaled =
//             (float)(x - inputMin) / (inputMax - inputMin);

//         int y = outputMin + scaled * (outputMax - outputMin);

//         normalized.axis[i] = y;
//     }

//     return normalized;
// }

// int main()
// {
//     MyActuator robot(NUM_MOTORS, "/dev/ttyUSB0");

//     // std::thread statusThread(printStatus, std::ref(robot));
//     bool prev_buttons[12] = {false};
//     Joystick js;
//     bool tryal = true;
//     bool isJoystick = false;

//     if (!js.isConnected())
//         return 1;

//     bool prev_button4 = false;

//     int lastAxis3 = 0;
//     bool isMoterRunning = false;

//     int lastAxis0 = 0;
//     bool isMoter2Running = false;
//     while (robotRunningState)
//     {
//         js.poll();
//         rawState = js.getState();

//         // Normilize the Asix value from -50 to 50;
//         state = normalizeJoyState(rawState); // NORMALIZED

//         // Shutdown
//         // printState(state);
//         if (state.button[4] && state.button[5])
//         {
//             robotRunningState = false;
//         }

//         // Edge detection for toggle
//         bool curr_button4 = state.button[4];

//         if (curr_button4 && !prev_button4)
//         {
//             robot.stop(1);
//             isJoystick = !isJoystick;
//             std::cout << "Joystick Enabled: " << isJoystick << std::endl;
//         }

//         prev_button4 = curr_button4;

//         // Use the flag
//         if (isJoystick)
//         {

//             for (int i = 0; i < 4; i++)
//             {
//                 bool curr = state.button[i];

//                 // Rising edge detection
//                 if (curr && !prev_buttons[i])
//                 {
//                     switch (i)
//                     {
//                     case 0:
//                         robot.setTourqe(1, 3);
//                         break;

//                     case 1:
//                         robot.emergencyCommand(2);
//                         break;

//                     case 2:
//                         robot.moveToPosition(23, 4);
//                         break;

//                     case 3:
//                         robot.setAcc(32, 2);
//                         break;
//                     }
//                 }

//                 prev_buttons[i] = curr;
//             }

//             if (lastAxis3 != state.axis[3])
//             {
//                 robot.moveAtSpeed(205, state.axis[3]);
//                 // robot.moveAtSpeed(2, state.axis[3]);

//                 lastAxis3 = state.axis[3];
//                 isMoterRunning = true;
//             }
//             else if (isMoterRunning && state.axis[3] == 0)
//             {
//                 robot.stop(1);
//                 // robot.stop(2);

//                 isMoterRunning = false;
//             }

//             // Motor 2

//             // if (lastAxis0 != state.axis[0])
//             // {
//             //     robot.moveAtSpeed(2, state.axis[0]);
//             //     lastAxis0 = state.axis[0];
//             //     isMoter2Running = true;
//             // }
//             // else if (isMoter2Running && state.axis[0] == 0)
//             // {
//             //     robot.stop(2);
//             //     isMoter2Running = false;
//             // }

//             usleep(10000);
//         }
//     }
//     return 0;
// }

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

    actuator.start();

    bool speedMode = true;

    bool prevBtn[12] = {};

    using Clock = std::chrono::steady_clock;
    auto nextWake = Clock::now();
    bool isJoystickActive = false;
    bool isMotorInit = false;

    while (!g_shutdown.load())
    {
        joy.poll();
        const JoyCommand &js = joy.getState();

        bool cur[12];
        for (int i = 0; i < 12; ++i)
            cur[i] = js.button[i];

        if (risingEdge(cur[1], prevBtn[1]))
        {
            std::cout << " Joy " << isJoystickActive << "\n";
            isJoystickActive = !isJoystickActive;
            if (isJoystickActive == 0)
                isMotorInit = 0;
        }
        if (risingEdge(cur[5], prevBtn[5]))
        {
            actuator.stop();
            exit(0);
        }

        if (isJoystickActive)
        {

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



            std::cout << js.axis[3] << std::endl;
        }
    }

    return 0;
}