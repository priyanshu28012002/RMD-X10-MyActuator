#include <iostream>
#include <iomanip>
#include <array>
#include "Crc16.hpp"

constexpr size_t FRAME_SIZE = 13;
constexpr size_t CRC_START  = 1;   // exclude header
constexpr size_t CRC_LEN    = 10;  // ID + LEN + DATA

void printFrame(const std::array<uint8_t, FRAME_SIZE>& frame)
{
    for (auto b : frame)
    {
        std::cout << std::hex
                  << std::setw(2)
                  << std::setfill('0')
                  << static_cast<int>(b) << " ";
    }
    std::cout << std::dec << "\n";
}

bool validateFrame(const std::array<uint8_t, FRAME_SIZE>& frame)
{
    uint16_t received =frame[11] | (frame[12] << 8);

    uint16_t calculated =  Crc16::compute(&frame[CRC_START], CRC_LEN);

    return received == calculated;
}

int main()
{
    std::array<uint8_t, FRAME_SIZE> frame{};

    // -------- Case 1: Build Valid Frame --------
    frame[0] = 0x3E;  // Header
    frame[1] = 0x01;  // ID
    frame[2] = 0x08;  // Length

    frame[3] = 0x60;  // Command
    for (int i = 4; i <= 10; ++i)frame[i] = 0x00;

    uint16_t crc = Crc16::compute(&frame[CRC_START], CRC_LEN);

    frame[11] = crc & 0xFF;         // CRC Low
    frame[12] = (crc >> 8) & 0xFF;  // CRC High

    std::cout << "Valid Frame:\n";
    printFrame(frame);

    // -------- Case 2: Validate --------
    if (validateFrame(frame))
        std::cout << "CRC VALID\n";
    else
        std::cout << "CRC INVALID\n";

    // -------- Case 3: Corrupt Frame --------
    frame[5] = 0xFF;

    if (validateFrame(frame))
        std::cout << "CRC VALID (unexpected)\n";
    else
        std::cout << "CRC INVALID (corruption detected)\n";

    // -------- Case 4: Known Test Vector --------
    const uint8_t test_data[] =
        {'1','2','3','4','5','6','7','8','9'};

    uint16_t test_crc =
        Crc16::compute(test_data, 9);

    std::cout << "CRC for 123456789: 0x"
              << std::hex << test_crc
              << std::dec << "\n";

    return 0;
}