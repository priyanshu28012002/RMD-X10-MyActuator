#pragma once
#ifndef _COMMON_SERIAL_COMM_H_
#define _COMMON_SERIAL_COMM_H_

#include <string>
#include <vector>
#include <stdint.h>

#include "SerialPort.hpp"

struct SerialConfig
{
    std::string port = "/dev/ttyUSB0";
    BaudRateType baudRateType = BaudRateType::STANDARD;
    BaudRate baudRate = BaudRate::B_115200;
    Parity parity = Parity::NONE;
    NumDataBits dataBits = NumDataBits::EIGHT;
    NumStopBits stopBits = NumStopBits::ONE;
    uint32_t timeout = 100;
};

class SerialComm
{
protected:
private:
    std::string port_;
    unsigned int baudrate_;
    unsigned int timeout_;
    std::string receive_data_;

public:
    SerialComm();
    ~SerialComm();

    void get_port(std::string &port);
    void serial_init();
    void serial_shutdown();

    // string
    void send_serial_s(const std::string &send);
    void receive_serial_s(std::string &receive);
    void send_receive_serial_s(const std::string &send, std::string &receive);

    // binary
    void send_serial_b(std::vector<uint8_t> &send);
    void receive_serial_b(std::vector<uint8_t> &receive);
    void send_receive_serial_b(std::vector<uint8_t> &send, std::vector<uint8_t> &receive);
    SerialPort serialPort_;
};

#endif //_COMMON_SERIAL_COMM_H_
