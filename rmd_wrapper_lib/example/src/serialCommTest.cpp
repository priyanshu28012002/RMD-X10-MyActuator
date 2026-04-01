#include <iostream>
#include <string>
#include <vector>
#include "SerialComm.hpp"

int main()
{
    try
    {
        SerialComm serial;

       
        std::string port = "/dev/ttyUSB0";
        serial.get_port(port);

        // Initialize serial communication
        serial.serial_init();

        std::cout << "Serial port opened successfully.\n";

        // ----------------------------
        // STRING COMMUNICATION
        // ----------------------------
        std::string send_data = "Hello from PC\n";
        std::string receive_data;

        serial.send_receive_serial_s(send_data, receive_data);

        std::cout << "Received: " << receive_data << std::endl;

        // ----------------------------
        // BINARY COMMUNICATION
        // ----------------------------
        std::vector<uint8_t> send_bin = {0x01, 0x02, 0x03, 0x04};
        std::vector<uint8_t> receive_bin;

        serial.send_receive_serial_b(send_bin, receive_bin);

        std::cout << "Received Binary: ";
        for (auto byte : receive_bin)
        {
            std::cout << std::hex << (int)byte << " ";
        }
        std::cout << std::endl;

        // Shutdown
        serial.serial_shutdown();
        std::cout << "Serial port closed.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}