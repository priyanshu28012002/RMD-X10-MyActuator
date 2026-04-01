#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>
#include <cstring>

struct JoyCommand
{
    bool button[12] = {0};
    int axis[6] = {0};
};

class Joystick
{
private:
    int js_fd;
    std::string device;
    JoyCommand state;

public:
    Joystick(const std::string& dev = "/dev/input/js0")
        : device(dev), js_fd(-1)
    {
        openDevice();
            resetState(); 

    }

    ~Joystick()
    {
        closeDevice();
    }

    bool openDevice()
    {
        js_fd = open(device.c_str(), O_RDONLY | O_NONBLOCK);
        if (js_fd < 0)
        {
            perror("Failed to open joystick");
            return false;
        }

        std::cout << "Joystick connected: " << device << std::endl;
        return true;
    }

    void closeDevice()
    {
        if (js_fd >= 0)
        {
            close(js_fd);
            js_fd = -1;
        }
    }

    bool isConnected() const
    {
        return js_fd >= 0;
    }

    // Poll new events (non-blocking)
    void poll()
{
    if (js_fd < 0) return;

    js_event event;

    while (read(js_fd, &event, sizeof(event)) > 0)
    {
        if (event.type & JS_EVENT_INIT)
            continue;

        if (event.type == JS_EVENT_BUTTON)
        {
            if (event.number < 12)
                state.button[event.number] = event.value;
        }
        else if (event.type == JS_EVENT_AXIS)
        {
            if (event.number < 6)
                state.axis[event.number] = event.value;
        }
    }
}

void resetState()
{
    memset(&state, 0, sizeof(state));
}

    const JoyCommand& getState() const
    {
        return state;
    }

    void printState() const
    {
        std::cout << "Buttons: ";
        for (int i = 0; i < 12; i++)
            std::cout << state.button[i] << " ";

        std::cout << "\nAxes: ";
        for (int i = 0; i < 6; i++)
            std::cout << state.axis[i] << " ";

        std::cout << "\n---------------------\n";
    }
};