
#include "x10_api.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <stdio.h>
#include <unistd.h>

#define MOTOR_ID (2)
#define PORT_ADDR "/dev/ttyUSB0"

void api_serial(void);

std::string port = PORT_ADDR;
uint8_t id = MOTOR_ID;

// 2.1
void Motor_read_pid(X10ApiSerial *xobj, uint8_t id)
{
    uint8_t buffer[8] = {0};

    int8_t result = xobj->Motor_read_pid(id, buffer);

    std::cout << "Motor_read_pid Result: " << (int)result << std::endl;

    if (result != 0)
    {
        std::cout << "Read failed\n";
        return;
    }

    for (int i = 0; i < 8; i++)
    {
        std::cout << (int)buffer[i] << " " << std::endl;
    }
}

void Motor_write_pid(X10ApiSerial *xobj, uint8_t id)
{
    // uint8_t buffer[8] = {0};

    // int8_t result = xobj->Motor_read_pid(id, buffer);

    // std::cout << "Motor_read_pid Result: " << (int)result << std::endl;

    // if (result != 0)
    // {
    //     std::cout << "Read failed\n";
    //     return;
    // }

    // for (int i = 0; i < 8; i++)
    // {
    //     std::cout << (int)buffer[i] << " " << std::endl;
    // }
    int8_t result = xobj->Write_pid_ROM(2, 100, 100, 100, 5, 100, 5);
    std::cout << "Motor_read_pid Result: " << (int)result << std::endl;
}

// 2.4
void Motor_read_accel(X10ApiSerial *xobj, uint8_t id)
{

    int32_t get_accel = 0;
    int8_t x = xobj->Motor_read_accel(id, get_accel);
    std::cout << "Motor_read_accel Result " << (int)x << std::endl;
    std::cout << "Acc : " << (int)get_accel << std::endl;
}
// 2.6
void multi_turn_current_pos(X10ApiSerial *xobj, uint8_t id)
{
    int32_t get_current_enc_pose = 0;
    int8_t x = xobj->multi_turn_current_pos(id, get_current_enc_pose);
    std::cout << "multi_turn_current_pos Result " << (int)x << std::endl;
    std::cout << "get_current_enc_pose : " << (int)get_current_enc_pose << std::endl;
}
// 2.7
void multi_turn_original_pos(X10ApiSerial *xobj, uint8_t id)
{
    int32_t get_original_enc_pose = 0;
    int8_t x = xobj->multi_turn_original_pos(id, get_original_enc_pose);
    std::cout << "multi_turn_original_pos Result " << (int)x << std::endl;
    std::cout << "get_original_enc_pose : " << (int)get_original_enc_pose << std::endl;
}
// 2.8
void multi_turn_zero_off_pos(X10ApiSerial *xobj, uint8_t id)
{
    int32_t get_zero_off_pose = 0;
    int8_t x = xobj->multi_turn_zero_off_pos(id, get_zero_off_pose);
    std::cout << "multi_turn_zero_off_pos Result " << (int)x << std::endl;
    std::cout << "get_zero_off_pose : " << (int)get_zero_off_pose << std::endl;
}
// 2.11
void get_single_enc(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t x = xobj->get_single_enc(id, buffer);
    std::cout << "get_single_enc Result " << (int)x << std::endl;
    for (int i = 0; i < 16; i++)
    {
        std::cout << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
}

// 2.12
void get_multi_angle(X10ApiSerial *xobj, uint8_t id)
{
    int32_t motorAngle = 0;
    int8_t x = xobj->get_multi_angle(id, motorAngle);
    std::cout << "get_multi_angle Result " << (int)x << std::endl;
    std::cout << "motorAngle : " << (int)motorAngle << std::endl;
}
// 2.13
void get_single_angle(X10ApiSerial *xobj, uint8_t id)
{
    int32_t motorAngle = 0;
    int8_t x = xobj->get_single_angle(id, motorAngle);
    std::cout << "get_single_angle Result " << (int)x << std::endl;
    std::cout << "motorAngle : " << (int)motorAngle << std::endl;
}
// 2.14
void Motor_state1(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t x = xobj->Motor_state1(id, buffer);
    std::cout << "Motor_state1 Result " << (int)x << std::endl;
    for (int i = 0; i < 16; i++)
    {
        std::cout << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
}
// 2.15
void Motor_state2(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t x = xobj->Motor_state2(id, buffer);
    std::cout << "Motor_state2 Result " << (int)x << std::endl;
    for (int i = 0; i < 16; i++)
    {
        std::cout << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
}
// 2.16
void Motor_state3(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t x = xobj->Motor_state3(id, buffer);
    std::cout << "Motor_state3 Result " << (int)x << std::endl;
    for (int i = 0; i < 16; i++)
    {
        std::cout << (int)buffer[i] << " ";
    }
    std::cout << std::endl;
}
// 2.17
void Motor_shut_down(X10ApiSerial *xobj, int id)
{
    int8_t x = xobj->Motor_stop(id);
    std::cout << "Motor_shut_down Result " << (int)x << std::endl;
}
// 2.18
void Motor_stop(X10ApiSerial *xobj, uint8_t id)
{

    int8_t x = xobj->Motor_stop(id);
    std::cout << "Motor_stop Result" << x << std::endl;
}

// 2.19
void torqueControl(X10ApiSerial *xobj, uint8_t id)
{

    int8_t x = xobj->torqueControl(id, 20);
    std::cout << "torqueControl Result " << (int)x << std::endl;
}
// 2.20
void speedControl(X10ApiSerial *xobj, uint8_t id, int speed)
{

    int8_t x = xobj->speedControl(id, speed);
    std::cout << "speedControl Result " << (int)x << std::endl;
}
int8_t speedControl(uint8_t id, int32_t speed, int16_t &get_speed); // 2.20
// int8_t abs_pose_control(uint8_t id, uint16_t maxSpeed, int32_t angle);              // 2.21

// 2.21
void abs_pose_control(X10ApiSerial *xobj, uint8_t id)
{

    int8_t x = xobj->abs_pose_control(id, 2000, 3000);
    std::cout << "abs_pose_control Result " << (int)x << std::endl;
}

// 2.22
void single_turn(X10ApiSerial *xobj, uint8_t id)
{
    uint8_t dir = 0x00;
    int8_t x = xobj->single_turn(id, dir, 2000, 3000);
    std::cout << "single_turn Result " << (int)x << std::endl;

    sleep(5);
    dir = 0x01;
    x = xobj->single_turn(id, dir, 2000, 3000);
    std::cout << "single_turn Result " << (int)x << std::endl;
}

// 2.23
void increment_control(X10ApiSerial *xobj, uint8_t id, uint16_t maxSpeed, int32_t angle)
{

    int8_t x = xobj->increment_control(id, maxSpeed, angle);
    std::cout << "increment_control Result " << x << std::endl;
}
// 7.0a
void set_Motor_id(X10ApiSerial *xobj, uint8_t newID)
{
    int8_t x = xobj->set_Motor_id(0xCD, newID);
    std::cout << "set_Motor_id Result" << x << std::endl;
}

int main()
{
    int t1 = clock();

    std::cout << "start time: " << t1 << std::endl;

    X10ApiSerial *xobj;
    xobj = new X10ApiSerial();

    xobj->get_port_address(port);
    xobj->rmdX10_init();

    // Motor_state1(xobj, 1);
    // sleep(10);
    // Motor_state2(xobj, 1);
    // sleep(10);
    // Motor_state3(xobj, 1);
    // set_Motor_id(xobj, 1);
    speedControl(xobj, 1, 5000); // Motor_state1(xobj, 2);
    sleep(1);
    speedControl(xobj, 2, 5000); // Motor_state1(xobj, 2);
    sleep(10);
    speedControl(xobj, 1, 0); // Motor_state1(xobj, 2);
    sleep(1);
    speedControl(xobj, 2, 0); // Motor_state1(xobj, 2);
    // Motor_state2(xobj, 2);
    // Motor_state3(xobj, 2);
    return 0;
}
