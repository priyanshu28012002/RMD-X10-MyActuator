/**
 * -------------------------------------------- COPYRIGHT NOTICE ---------------------------------------------------\n
 * Copyright (C) 2023 Octobotics Tech Pvt. Ltd. All Rights Reserved.\n
 * Do not remove this copyright notice.\n
 * Do not use, reuse, copy, merge, publish, sub-license, sell, distribute or modify this code - except without\n
 * explicit, written permission from Octobotics Tech Pvt. Ltd.\n
 * Contact connect@octobotics.tech for full license information.\n
 * -------------------------------------------- COPYRIGHT NOTICE ---------------------------------------------------
 *
 *
 * @file serial_comm.cpp
 * @author vaibhav
 * @brief
 * @date 2023-04-07
 *
 *
 */


#include <serial_comm.h>

SerialComm::SerialComm()
{
    port_ = "/dev/ttyUSB0";
    baudrate_ = 115200;
    timeout_ = 100;
}


SerialComm::~SerialComm()
{
    serialPort_.Close();
}

void SerialComm::get_port(std::string &port)
{
    port_ = port;
    timeout_ = 100;
}

void SerialComm::serial_init()
{
    serialPort_ = SerialPort(port_, BaudRate::B_115200, NumDataBits::EIGHT, Parity::NONE, NumStopBits::ONE);
    serialPort_.SetTimeout(timeout_); // Block for up to 100ms to receive data
    serialPort_.Open();
}

void SerialComm::serial_shutdown()
{
    serialPort_.Close();
}

void SerialComm::send_serial_s(const std::string &data)
{
    serialPort_.Write(data);
}

void SerialComm::receive_serial_s(std::string &data)
{
    serialPort_.Read(data);
}

void SerialComm::send_receive_serial_s(const std::string &send, std::string &receive)
{
    send_serial_s(send);
    receive_serial_s(receive);
}

void SerialComm::send_serial_b(std::vector<uint8_t> &send)
{
    serialPort_.WriteBinary(send);
}
void SerialComm::receive_serial_b(std::vector<uint8_t> &receive)
{
    serialPort_.ReadBinary(receive);
}

void SerialComm::send_receive_serial_b(std::vector<uint8_t> &send, std::vector<uint8_t> &receive)
{
    send_serial_b(send);
    receive_serial_b(receive);
}
