#include "x10_api.hpp"

#include <iostream>
#include <unistd.h>
#include <cstdio>
#include <stdio.h>
#include <unistd.h>

#define MOTOR_ID (2)
#define PORT_ADDR "/dev/ttyUSB0"

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
    int8_t result = xobj->Write_pid_ROM(2, 100, 100, 100, 5, 100, 5);
    std::cout << "Motor_write_pid Result: " << (int)result << std::endl;
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
void parseMoterState1(int16_t* buffer)
{
    // Each buffer element holds one byte from the DATA[] reply frame.
    // Cast to uint8_t to safely extract byte values.
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);

    int8_t temperature = (int8_t)data[1]; // 1 °C / LSB
    uint8_t brakeState = data[3];         // 1 = released, 0 = locked
    uint16_t voltage_raw = (uint16_t)data[4] | ((uint16_t)data[5] << 8);
    float voltage = voltage_raw * 0.1f; // 0.1 V / LSB
    uint16_t errorState = (uint16_t)data[6] | ((uint16_t)data[7] << 8);

    std::cout << "Temperature     : " << (int)temperature << " °C" << std::endl;
    std::cout << "Brake State     : " << (brakeState ? "Released (1)" : "Locked (0)") << std::endl;
    std::cout << "Voltage         : " << voltage << " V" << std::endl;
    std::cout << "Error Flags     : 0x" << std::hex << errorState << std::dec << std::endl;

    // Decode individual error bits (refer to protocol section 2.14 / Failure Description)
    if (errorState == 0)
    {
        std::cout << "  No errors detected." << std::endl;
    }
    else
    {
        if (errorState & (1 << 0))
            std::cout << "  [bit0] Motor stall" << std::endl;
        if (errorState & (1 << 1))
            std::cout << "  [bit1] Low voltage" << std::endl;
        if (errorState & (1 << 2))
            std::cout << "  [bit2] Over voltage" << std::endl;
        if (errorState & (1 << 3))
            std::cout << "  [bit3] Over current" << std::endl;
        if (errorState & (1 << 4))
            std::cout << "  [bit4] Power overrun" << std::endl;
        if (errorState & (1 << 5))
            std::cout << "  [bit5] Speeding" << std::endl;
        if (errorState & (1 << 6))
            std::cout << "  [bit6] Over temperature" << std::endl;
        if (errorState & (1 << 7))
            std::cout << "  [bit7] Encoder calibration error" << std::endl;
    }
    std::cout << std::endl;
}

void parseMoterState2(int16_t* buffer)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);

    int8_t temperature = (int8_t)data[1];
    int16_t iq_raw = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
    float iq = iq_raw * 0.01f;                                                       // 0.01 A / LSB
    int16_t speed = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));         // 1 dps / LSB
    int16_t encoder_angle = (int16_t)((uint16_t)data[6] | ((uint16_t)data[7] << 8)); // 1 degree / LSB

    std::cout << "Temperature     : " << (int)temperature << " °C" << std::endl;
    std::cout << "Torque Current  : " << iq << " A" << std::endl;
    std::cout << "Speed           : " << (int)speed << " dps" << std::endl;
    std::cout << "Encoder Angle   : " << (int)encoder_angle << " degree" << std::endl;
    std::cout << std::endl;
}

void parseMoterState3(int16_t* buffer)
{
    uint8_t *data = reinterpret_cast<uint8_t *>(buffer);

    int8_t temperature = (int8_t)data[1];
    int16_t iA_raw = (int16_t)((uint16_t)data[2] | ((uint16_t)data[3] << 8));
    int16_t iB_raw = (int16_t)((uint16_t)data[4] | ((uint16_t)data[5] << 8));
    int16_t iC_raw = (int16_t)((uint16_t)data[6] | ((uint16_t)data[7] << 8));

    float iA = iA_raw * 0.01f; // 0.01 A / LSB
    float iB = iB_raw * 0.01f;
    float iC = iC_raw * 0.01f;

    std::cout << "Temperature     : " << (int)temperature << " °C" << std::endl;
    std::cout << "Phase A Current : " << iA << " A" << std::endl;
    std::cout << "Phase B Current : " << iB << " A" << std::endl;
    std::cout << "Phase C Current : " << iC << " A" << std::endl;
    std::cout << std::endl;
}

// ============================================================
// 2.14 - Read Motor Status 1 and Error Flag Command (0x9A)
//
// Reply buffer layout (each index = one byte from DATA[]):
//   DATA[0] : Command byte (0x9A)
//   DATA[1] : Motor temperature      -> int8_t,  unit: 1 °C/LSB
//   DATA[2] : NULL
//   DATA[3] : Brake release command  -> uint8_t, 1 = brake released, 0 = brake locked
//   DATA[4] : Voltage low byte       -> uint16_t (DATA[4] | DATA[5]<<8), unit: 0.1 V/LSB
//   DATA[5] : Voltage high byte
//   DATA[6] : Error flag low byte    -> uint16_t (DATA[6] | DATA[7]<<8), each bit = error state
//   DATA[7] : Error flag high byte
// ============================================================
void Motor_state1(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t result = xobj->Motor_state1(id, buffer);

    std::cout << "\n--- Motor_state1 (0x9A) | Motor ID: " << (int)id << " ---" << std::endl;
    std::cout << "API call result : " << (int)result << std::endl;

    if (result != 0)
    {
        std::cout << "Read failed." << std::endl;
        return;
    }
    parseMoterState1(buffer);

    
}

// ============================================================
// 2.15 - Read Motor Status 2 Command (0x9C)
//
// Reply buffer layout:
//   DATA[0] : Command byte (0x9C)
//   DATA[1] : Motor temperature      -> int8_t,  unit: 1 °C/LSB
//   DATA[2] : Torque current iq low  -> int16_t (DATA[2] | DATA[3]<<8), unit: 0.01 A/LSB
//   DATA[3] : Torque current iq high
//   DATA[4] : Speed low byte         -> int16_t (DATA[4] | DATA[5]<<8), unit: 1 dps/LSB
//   DATA[5] : Speed high byte
//   DATA[6] : Encoder angle low byte -> int16_t (DATA[6] | DATA[7]<<8), unit: 1 degree/LSB, range ±32767°
//   DATA[7] : Encoder angle high byte
// ============================================================
void Motor_state2(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t result = xobj->Motor_state2(id, buffer);

    std::cout << "\n--- Motor_state2 (0x9C) | Motor ID: " << (int)id << " ---" << std::endl;
    std::cout << "API call result : " << (int)result << std::endl;

    if (result != 0)
    {
        std::cout << "Read failed." << std::endl;
        return;
    }
        parseMoterState2(buffer);
}






// ============================================================
// 2.16 - Read Motor Status 3 Command (0x9D)
//
// Reply buffer layout:
//   DATA[0] : Command byte (0x9D)
//   DATA[1] : Motor temperature          -> int8_t,  unit: 1 °C/LSB
//   DATA[2] : Phase A current low byte   -> int16_t (DATA[2] | DATA[3]<<8), unit: 0.01 A/LSB
//   DATA[3] : Phase A current high byte
//   DATA[4] : Phase B current low byte   -> int16_t (DATA[4] | DATA[5]<<8), unit: 0.01 A/LSB
//   DATA[5] : Phase B current high byte
//   DATA[6] : Phase C current low byte   -> int16_t (DATA[6] | DATA[7]<<8), unit: 0.01 A/LSB
//   DATA[7] : Phase C current high byte
// ============================================================
void Motor_state3(X10ApiSerial *xobj, uint8_t id)
{
    int16_t buffer[16] = {0};
    int8_t result = xobj->Motor_state3(id, buffer);

    std::cout << "\n--- Motor_state3 (0x9D) | Motor ID: " << (int)id << " ---" << std::endl;
    std::cout << "API call result : " << (int)result << std::endl;

    if (result != 0)
    {
        std::cout << "Read failed." << std::endl;
        return;
    }

    parseMoterState3(buffer);
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
    std::cout << "Motor_stop Result " << (int)x << std::endl;
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
    std::cout << "increment_control Result " << (int)x << std::endl;
}

// 7.0a
void set_Motor_id(X10ApiSerial *xobj, uint8_t newID)
{
    int8_t x = xobj->set_Motor_id(0xCD, newID);
    std::cout << "set_Motor_id Result " << (int)x << std::endl;
}

int main()
{
    int t1 = clock();
    std::cout << "start time: " << t1 << std::endl;

    X10ApiSerial *xobj;
    xobj = new X10ApiSerial();

    xobj->get_port_address(port);
    xobj->rmdX10_init();

    while (true)
    {
        // Read all state info for Motor ID 1
        std::cout << "\n====== Motor ID 1 ======" << std::endl;
        Motor_state1(xobj, 1); // Temperature, Voltage, Brake, Error flags
        Motor_state2(xobj, 1); // Temperature, Torque current, Speed, Encoder angle
        Motor_state3(xobj, 1); // Temperature, Phase A/B/C currents

        // Read all state info for Motor ID 2
        std::cout << "\n====== Motor ID 2 ======" << std::endl;
        Motor_state1(xobj, 2);
        Motor_state2(xobj, 2);
        Motor_state3(xobj, 2);
        sleep(1);
    }

    delete xobj;
    return 0;
}