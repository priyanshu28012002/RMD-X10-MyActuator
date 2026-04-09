#include <iostream>
#include <chrono>
#include <stdexcept>
#include <unistd.h>


class Printer
{
private:
    std::chrono::steady_clock::time_point last_call;

public:
    Printer()
    {
        last_call = std::chrono::steady_clock::now() - std::chrono::milliseconds(2);
    }

    void print()
    {
        auto now = std::chrono::steady_clock::now();
        auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(now - last_call);

        if (diff.count() < 2)
        {
            throw std::runtime_error("print() called too fast! Minimum interval is 2ms");
        }

        last_call = now;
        std::cout << "Printing safely...\n";
    }
};

int main()
{
    Printer p;
    p.print();
    usleep(2000);
    p.print();
    usleep(2000);
    p.print();
    return 0;
}