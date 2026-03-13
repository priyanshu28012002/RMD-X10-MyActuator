#ifndef CRC16_HPP
#define CRC16_HPP

#include <cstdint>
#include <vector>

class Crc16
{
public:
    // Compute CRC16 (Modbus / IBM)
    static uint16_t compute(const uint8_t* data, size_t length)
    {
        uint16_t crc = 0xFFFF;

        for (size_t i = 0; i < length; ++i)
        {
            crc ^= data[i];

            for (int bit = 0; bit < 8; ++bit)
            {
                if (crc & 0x0001)
                {
                    crc >>= 1;
                    crc ^= 0xA001;
                }
                else
                {
                    crc >>= 1;
                }
            }
        }

        return crc;
    }

    static uint16_t compute(const std::vector<uint8_t>& data)
    {
        return compute(data.data(), data.size());
    }

    static bool validate(const std::vector<uint8_t>& frame)
    {
        if (frame.size() < 3)
            return false;

        // Last 2 bytes = CRC
        size_t data_len = frame.size() - 2;

        uint16_t received =
            frame[data_len] |
            (frame[data_len + 1] << 8);

        uint16_t calculated =
            compute(frame.data(), data_len);

        return received == calculated;
    }

    static void append(std::vector<uint8_t>& frame)
    {
        uint16_t crc = compute(frame);
        frame.push_back(crc & 0xFF);         // Low byte
        frame.push_back((crc >> 8) & 0xFF);  // High byte
    }
};

#endif