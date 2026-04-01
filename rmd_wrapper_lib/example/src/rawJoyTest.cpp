#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <linux/joystick.h>

struct joyCommand
{
    bool button[12]={0};
    int axis[6]={0};
};


int main() {
    const char* device = "/dev/input/js0";

    int js = open(device, O_RDONLY);
    if (js < 0) {
        perror("Failed to open joystick");
        return 1;
    }

    std::cout << "Listening to joystick on " << device << "...\n";

    js_event event;

    int count = 0;
    while (true) {
        
        ssize_t bytes = read(js, &event, sizeof(event));

        if (bytes == sizeof(event)) {
            event.type &= ~JS_EVENT_INIT; // remove init flag

            // Save the value and flash the vlaue

        }
      
    }

    close(js);
    return 0;
}