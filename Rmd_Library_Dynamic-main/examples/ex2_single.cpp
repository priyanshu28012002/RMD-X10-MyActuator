/**
 * -----------------------------------------— COPYRIGHT NOTICE ------------------------------------------------\n
 * Copyright (C) 2023 Octobotics Tech Pvt. Ltd. All Rights Reserved.\n
 * Do not remove this copyright notice.\n
 * Do not use, reuse, copy, merge, publish, sub-license, sell, distribute or modify this \n
 * code - except without explicit, written permission from Octobotics Tech Pvt. Ltd.\n
 * Contact connect@octobotics.tech for full license information.\n
 * -----------------------------------------— COPYRIGHT NOTICE ------------------------------------------------\n
 *
 *
 * @file ex1_info.cpp
 * @author vaibhav (vaibhav.s@octobotics.tech)
 * @brief
 * @date 2023-05-03
 *
 *
 */


#include "x10_api.h"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <stdio.h>


#define MOTOR_ID        (1)
#define PORT_ADDR        "/dev/ttyUSB0"


void api_serial(void);

std::string port = PORT_ADDR;
uint8_t id = MOTOR_ID;

int main()
{
    int t1 = clock();

    std::cout << "start time: " << t1 << std::endl;
    std::cout << "------------------------------------\n";
    std::cout << "|   I am here in ex2_single.cpp!   |\n";
    std::cout << "------------------------------------\n";
    api_serial();



    return 0;
}


/**
 * @brief initialize the serial port and read the actuator status
 * @param
 */
void api_serial(void)
{
    int8_t ret = 0;
    uint8_t mode;

    X10ApiSerial *xobj;
    xobj = new X10ApiSerial();

    xobj->get_port_address(port);
    xobj->rmdX10_init();

    // xobj->increment_control(1,60,36000);
    // xobj->increment_control(2,60,36000);
    // xobj->increment_control(3,60,36000);

    xobj->increment_control(0xCD, 200, 3*36000);
    sleep(1);

    xobj->increment_control(0xCD, 200, -3*36000);
    sleep(1);

    xobj->increment_control(0xCD, 200, 9000);
    sleep(1);

    xobj->increment_control(0xCD, 200, -9000);
    sleep(1);

    xobj->increment_control(0xCD, 200, 1*36000);
    sleep(1);

}

//3 turn clockwise
//3 turn anticlockwise
//90
//90
