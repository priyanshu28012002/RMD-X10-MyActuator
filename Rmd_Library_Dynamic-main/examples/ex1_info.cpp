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


#define MOTOR_ID        (10)
#define PORT_ADDR        "/dev/ttyUSB0"


void print_pid(uint8_t *read_pid_arr);
void read_stat(uint8_t id, int8_t ret, X10ApiSerial *xobj);
void api_serial(void);

std::string port = PORT_ADDR;
uint8_t id = MOTOR_ID;

int main()
{
    int t1 = clock();

    std::cout << "start time: " << t1 << std::endl;
    std::cout << "------------------------------------\n";
    std::cout << "|     I am here in ex1_info.cpp!   |\n";
    std::cout << "------------------------------------\n";
    api_serial();

    int t2 = clock();

    std::cout << "end time: " << t2-t1 << std::endl;

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

    read_stat(id, ret, xobj);

     xobj->Motor_stop(id);
    ret = xobj->Motor_mode(id, mode);
    printf("%d | Stop_motor_mode: %d \n", ret, mode);
    xobj->Motor_shut_down(id);
    ret = xobj->Motor_mode(id, mode);
    printf("%d | Motor_shut_down_mode: %d \n", ret, mode);


}


/*
 // xobj->abs_pose_control(id, 50, 0);
    // xobj->abs_pose_control(id, 50, 360);
    int16_t get_speed;
    for (int i = 0; i < 100; i++)
    {
        xobj->speedControl(0x1, 1000, get_speed);
        printf("speed: %d \n", get_speed);
    }
    xobj->Motor_stop(id);
    ret = xobj->Motor_mode(id, mode);
    printf("%d | Stop_motor_mode: %d \n", ret, mode);
    xobj->Motor_shut_down(id);
    ret = xobj->Motor_mode(id, mode);
    printf("%d | Motor_shut_down_mode: %d \n", ret, mode);

    // xobj->Motor_reset(id);
    */



/**
 * @brief read the PID values
 * @param read_pid_arr
 */
void print_pid(uint8_t *read_pid_arr, int8_t ret)
{
    printf("Read PID:\n");
    printf("\tError---> %d \n", ret);
    printf("\tcurrKP--> %d \n", read_pid_arr[0]);
    printf("\tcurrKI--> %d \n", read_pid_arr[1]);
    printf("\tspdKP---> %d \n", read_pid_arr[2]);
    printf("\tspdKI---> %d \n", read_pid_arr[3]);
    printf("\tposKP---> %d \n", read_pid_arr[4]);
    printf("\tposKI---> %d \n", read_pid_arr[5]);
    printf("---------------------------\n");
}

/**
 * @brief read the actuator status
 * @param id
 * @param ret
 * @param xobj
 */
void read_stat(uint8_t id, int8_t ret, X10ApiSerial *xobj)
{
    uint8_t read_pid_arr[6] = { 0 };
    int16_t motor_stat1[4] = { 0 };
    int16_t motor_stat2[4] = { 0 };
    int16_t motor_stat3[4] = { 0 };

    int32_t pose = 0;
    // printf("get stat:\n");
    // read PID
    ret = xobj->Motor_read_pid(id, read_pid_arr);

    print_pid(read_pid_arr, ret);

    // read accel
    ret = xobj->Motor_read_accel(id, pose);
    printf("Motor_read_accel----------> %d | %d\n", ret, pose);

    // multi_turn_current_pos
    ret = xobj->multi_turn_current_pos(id, pose);
    printf("multi_turn_current_pos----> %d | %d\n", ret, pose);

    // multi_turn_original_pos
    ret = xobj->multi_turn_original_pos(id, pose);
    printf("multi_turn_original_pos---> %d | %d\n", ret, pose);

    // multi_turn_zero_off_pos
    ret = xobj->multi_turn_zero_off_pos(id, pose);
    printf("multi_turn_zero_off_pos---> %d | %d\n", ret, pose);
    printf("-------------------------------------------------------\n");

    // Motor_state1
    ret = xobj->Motor_state1(0x1, motor_stat1);
    // printf("\t%4d\t%4d\t %4d\t %0.2f\t %4d \n", ret, motor_stat1[0], motor_stat1[1], (float)motor_stat1[2] / 10.0, motor_stat1[3]); // motor_stat1
    printf("Motor state1:\n");
    printf("\tError----------> %d\n", ret);
    printf("\tTemperature----> %d\n", motor_stat1[0]);
    printf("\tBrake----------> %d\n", motor_stat1[1]);
    printf("\tVoltage--------> %0.2f\n", (float)motor_stat1[2] / 10.0);
    printf("\tErrorState-----> %d\n", motor_stat1[3]);
    printf("-------------------------------------------------------\n");


    // Motor_state2
    ret = xobj->Motor_state2(0x1, motor_stat2);
    // printf("%d | Motor state2: %4d\t %0.4f\t %4d\t %4d \n", ret, motor_stat2[0], (float)motor_stat2[1] / 100.00, motor_stat2[2], motor_stat2[3]); // motor_stat2
    printf("Motor state2:\n");
    printf("\tError----------> %d\n", ret);
    printf("\tTemperature----> %d\n", motor_stat2[0]);
    printf("\tTorqueCurrent--> %0.4f\n", (float)motor_stat2[1] / 100.00);
    printf("\tMotorSpeed-----> %d\n", motor_stat2[2]);
    printf("\tMotorAngle-----> %d\n", motor_stat2[3]);
    printf("-------------------------------------------------------\n");

    // Motor_state3
    ret = xobj->Motor_state3(0x1, motor_stat3);
    // printf("%d | Motor state3: %4d\t %4d\t %4d\t %4d \n", ret, motor_stat3[0], motor_stat3[1], motor_stat3[2], motor_stat3[3]); // motor_stat3
    printf("Motor state3:\n");
    printf("\tError----------> %d\n", ret);
    printf("\tTemperature----> %d\n", motor_stat3[0]);
    printf("\tI_phaseA-------> %d\n", motor_stat3[1]);
    printf("\tI_phaseB-------> %d\n", motor_stat3[2]);
    printf("\tI_phaseC-------> %d\n", motor_stat3[3]);
    printf("\tTotal current--> %d\n", motor_stat3[1] + motor_stat3[2] + motor_stat3[3]);
    printf("-------------------------------------------------------\n");
}
