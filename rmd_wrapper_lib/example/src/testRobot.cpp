#include "joystick.cpp"
#include "MyActuator.hpp"

constexpr int NUM_MOTORS = 1;

bool robotRunningState = true;

void printStatus(MyActuator &robot)
{
    while (robotRunningState)
    {
        std::cout << "\r"; // move cursor to start

        for (int i = 1; i <= NUM_MOTORS; i++)
        {
            Motor *m = robot.getMotor(i);
            if (!m)
                continue;

            std::cout << "[M" << i << "] ";

            std::cout << "T:" << m->getTemp() << " ";
            std::cout << "V:" << m->getVoltage() << " ";
            std::cout << "I:" << m->getCurrent() << " ";
            std::cout << "A:" << m->getAngle() << " | ";
        }

        std::cout << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

JoyCommand state;

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

int main()
{
    MyActuator robot(NUM_MOTORS, "/dev/ttyUSB0");

    std::thread statusThread(printStatus, std::ref(robot));
    bool prev_buttons[12] = {false};
    Joystick js;
    bool tryal = true;
    bool isJoystick = false;

    if (!js.isConnected())
        return 1;

    bool prev_button4 = false;

    int lastAxis3 = 0;
    while (robotRunningState)
    {
        js.poll();
        state = js.getState();

        // Shutdown
        if (state.button[4] && state.button[5])
        {
            robotRunningState = false;
        }

        // Edge detection for toggle
        bool curr_button4 = state.button[4];

        if (curr_button4 && !prev_button4)
        {
            isJoystick = !isJoystick;
            std::cout << "Joystick Enabled: " << isJoystick << std::endl;
        }

        prev_button4 = curr_button4;

        // Use the flag
        if (isJoystick)
        {

            for (int i = 0; i < 4; i++)
            {
                bool curr = state.button[i];

                // Rising edge detection
                if (curr && !prev_buttons[i])
                {
                    switch (i)
                    {
                    case 0:
                        robot.setTourqe(1, 3);
                        break;

                    case 1:
                        robot.emergencyCommand(2);
                        break;

                    case 2:
                        robot.moveToPosition(23, 4);
                        break;

                    case 3:
                        robot.setAcc(32, 2);
                        break;
                    }
                }

                prev_buttons[i] = curr;
            }

            if (lastAxis3 != state.axis[3])
            {
                robot.moveAtSpeed(1, state.axis[3]);
                lastAxis3 = state.axis[3];
            }
            else{
                robot.stop(1);
            }

            usleep(10000);
        }
    }
        return 0;
}
