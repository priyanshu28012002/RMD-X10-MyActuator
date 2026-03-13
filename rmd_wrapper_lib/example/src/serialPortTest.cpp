#include <iostream>
#include <vector>
#include <thread>
#include <chrono>
#include <SerialPort.hpp>

using namespace std;


int main()
{
    try
    {
        std::cout << "===== SERIAL PORT FULL FEATURE TEST =====\n";
        // Construct with STANDARD baud rate
        SerialPort sp("/dev/ttyUSB0",
                      BaudRate::B_115200,
                      NumDataBits::EIGHT,
                      Parity::NONE,
                      NumStopBits::ONE);

        sp.SetTimeout(100); // 100ms timeout
        sp.Open();

        std::cout << "Port opened successfully.\n";

        // Test Available()
        std::cout << "Bytes available initially: "
                  << sp.Available() << std::endl;

        // Write STRING
        std::string tx_msg = "Hello from PC\n";
        sp.Write(tx_msg);
        std::cout << "Sent string: " << tx_msg;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Read STRING
        std::string rx_msg;
        sp.Read(rx_msg);

        if (!rx_msg.empty())
            std::cout << "Received string: " << rx_msg << std::endl;
        else
            std::cout << "No string received.\n";

        // Write BINARY
        std::vector<uint8_t> tx_bin = {0x01, 0x02, 0xAA, 0x55};
        sp.WriteBinary(tx_bin);
        std::cout << "Sent binary: ";
        for (auto b : tx_bin)
            std::cout << std::hex << (int)b << " ";
        std::cout << std::dec << std::endl;

        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        // Read BINARY
        std::vector<uint8_t> rx_bin;
        sp.ReadBinary(rx_bin);

        if (!rx_bin.empty())
        {
            std::cout << "Received binary: ";
            for (auto b : rx_bin)
                std::cout << std::hex << (int)b << " ";
            std::cout << std::dec << std::endl;
        }
        else
        {
            std::cout << "No binary received.\n";
        }

        // Change configuration while OPEN
        std::cout << "Changing configuration...\n";

        sp.SetParity(Parity::EVEN);
        sp.SetNumDataBits(NumDataBits::SEVEN);
        sp.SetNumStopBits(NumStopBits::TWO);
        sp.SetEcho(true);

        std::cout << "Configuration changed to 7E2 + echo.\n";

        // Switch back to 8N1
        sp.SetParity(Parity::NONE);
        sp.SetNumDataBits(NumDataBits::EIGHT);
        sp.SetNumStopBits(NumStopBits::ONE);
        sp.SetEcho(false);

        std::cout << "Restored 8N1.\n";

        // Close and Re-open with CUSTOM baud
    
        sp.Close();
        std::cout << "Closed port.\n";

        SerialPort sp_custom("/dev/ttyUSB0", (speed_t)250000);
        sp_custom.SetTimeout(100);
        sp_custom.Open();

        std::cout << "Opened with custom baud 250000.\n";

        std::string custom_msg = "Custom Baud Test\n";
        sp_custom.Write(custom_msg);

        std::string custom_rx;
        sp_custom.Read(custom_rx);

        if (!custom_rx.empty())
            std::cout << "Custom RX: " << custom_rx << std::endl;

        // Final close
        sp_custom.Close();
        std::cout << "Test completed successfully.\n";
    }
    catch (const std::exception &e)
    {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

    return 0;
}