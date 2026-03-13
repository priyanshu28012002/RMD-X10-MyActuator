#include <x10_api.hpp>



X10ApiSerial::X10ApiSerial(/* args */)
{
    std::cout << "Constructor X10ApiSerial\n";

}

X10ApiSerial::~X10ApiSerial()
{        std::cout << "Disconstructor X10ApiSerial\n";

}

void X10ApiSerial::get_port_address(std::string &port)
{

    x10_api_base_.get_port_address(port);
}

void X10ApiSerial::rmdX10_init()
{
    x10_api_base_.rmdX10_init();
}

void X10ApiSerial::rmdX10_shut_down()
{
    x10_api_base_.rmdX10_shut_down();
}

    /****************Implemented***************/
int8_t X10ApiSerial::Motor_read_pid(uint8_t id_, uint8_t *data_arr)
{
    return x10_api_base_.Motor_read_pid(id_, data_arr);
}

int8_t X10ApiSerial::Write_pid_RAM(uint8_t id_, uint8_t CurrKP, uint8_t CurrKI, uint8_t SpdKP, uint8_t SpdKI, uint8_t PosKP, uint8_t PosKI)
{
    return x10_api_base_.Write_pid_RAM(id_, CurrKP, CurrKI, SpdKP, SpdKI, PosKP, PosKI);
}

int8_t X10ApiSerial::Write_pid_ROM(uint8_t id_, uint8_t CurrKP, uint8_t CurrKI, uint8_t SpdKP, uint8_t SpdKI, uint8_t PosKP, uint8_t PosKI)
{
    return x10_api_base_.Write_pid_ROM(id_, CurrKP, CurrKI, SpdKP, SpdKI, PosKP, PosKI);
}

int8_t X10ApiSerial::Motor_read_accel(uint8_t id_, int32_t &get_accel)
{
    return x10_api_base_.Motor_read_accel(id_, get_accel);
}

int8_t X10ApiSerial::Write_accel_ALL(uint8_t id, uint8_t index, uint32_t accel)                                                            // 2.5
{
    return x10_api_base_.Write_accel_ALL(id, index, accel);
}

int8_t X10ApiSerial::multi_turn_current_pos(uint8_t id_, int32_t &get_current_enc_pose)
{
    return x10_api_base_.multi_turn_current_pos(id_, get_current_enc_pose);
}

int8_t X10ApiSerial::multi_turn_original_pos(uint8_t id_, int32_t &get_original_enc_pose)
{
    return x10_api_base_.multi_turn_original_pos(id_, get_original_enc_pose);
}

int8_t X10ApiSerial::multi_turn_zero_off_pos(uint8_t id_, int32_t get_zero_off_pose)
{
    return x10_api_base_.multi_turn_zero_off_pos(id_, get_zero_off_pose);
}

int8_t X10ApiSerial::set_multi_turn_zero(uint8_t id_, int32_t offset)
{
    return x10_api_base_.set_multi_turn_zero(id_, offset);
}

int8_t X10ApiSerial::set_current_turn_zero(uint8_t id_) // 2.10
{
    return x10_api_base_.set_current_turn_zero(id_);
}
int8_t X10ApiSerial::get_single_enc(uint8_t id_, int16_t *data_arr) // 2.11
{
    return x10_api_base_.get_single_enc(id_, data_arr);
}
int8_t X10ApiSerial::get_multi_angle(uint8_t id_, int32_t motorAngle) // 2.12
{
    return x10_api_base_.get_multi_angle(id_, motorAngle);
}
int8_t X10ApiSerial::get_single_angle(uint8_t id_, int16_t motorAngle) // 2.13
{
    return x10_api_base_.get_single_angle(id_, motorAngle);
}

int8_t X10ApiSerial::Motor_state1(uint8_t id_, int16_t *data_arr)
{
    return x10_api_base_.Motor_state1(id_, data_arr);
}

int8_t X10ApiSerial::Motor_state2(uint8_t id_, int16_t *data_arr)
{
    return x10_api_base_.Motor_state2(id_, data_arr);
}

int8_t X10ApiSerial::Motor_state3(uint8_t id_, int16_t *data_arr)
{
    return x10_api_base_.Motor_state3(id_, data_arr);
}

int8_t X10ApiSerial::Motor_shut_down(uint8_t id_)
{
    return x10_api_base_.Motor_shut_down(id_);
}

int8_t X10ApiSerial::Motor_stop(uint8_t id_)
{
    return x10_api_base_.Motor_stop(id_);
}

int8_t X10ApiSerial::torqueControl(uint8_t id, int32_t iqControl) // 2.19
{
    return x10_api_base_.torqueControl(id, iqControl);
}

int8_t X10ApiSerial::speedControl(uint8_t id_, int32_t speed)
{
    return x10_api_base_.speedControl(id_, speed);
}

int8_t X10ApiSerial::speedControl(uint8_t id_, int32_t speed, int16_t &get_speed)
{
    return x10_api_base_.speedControl(id_, speed, get_speed);
}

int8_t X10ApiSerial::abs_pose_control(uint8_t id, uint16_t maxSpeed, int32_t angle)
{
    return x10_api_base_.abs_pose_control(id, maxSpeed, angle);
}

int8_t X10ApiSerial::single_turn(uint8_t id, uint8_t spinDir, uint16_t maxSpeed, uint16_t angle)
{
    return x10_api_base_.single_turn(id, spinDir, maxSpeed, angle);
}

int8_t X10ApiSerial::increment_control(uint8_t id, uint16_t maxSpeed, int32_t angle) // 2.23
{
    return x10_api_base_.increment_control(id, maxSpeed, angle);
}

int8_t X10ApiSerial::Motor_mode(uint8_t id, uint8_t &mode)
{
    return x10_api_base_.Motor_mode(id, mode);
}

int8_t X10ApiSerial::get_motor_power(uint8_t id) // 2.25
{
    return x10_api_base_.get_motor_power(id);
}

void X10ApiSerial::Motor_reset(uint8_t id)
{
    x10_api_base_.Motor_reset(id);
}

int8_t X10ApiSerial::Motor_runtime(uint8_t id, uint32_t &runTime)                                                                          // 2.29
{
    return x10_api_base_.Motor_runtime(id, runTime);
}
int8_t X10ApiSerial::Motor_edition(uint8_t id, uint32_t &sysDate)                                                                          // 2.30
{
    return x10_api_base_.Motor_edition(id, sysDate);
}
int8_t X10ApiSerial::Motor_comm_protect(uint8_t id, uint32_t commProtect)                                                                  // 2.31
{
    return x10_api_base_.Motor_comm_protect(id, commProtect);
}
void   X10ApiSerial::Motor_baudrate(uint8_t id, uint8_t baudrate)                                                                            // 2.32
{
    return x10_api_base_.Motor_baudrate(id, baudrate);
}
int8_t X10ApiSerial::Motor_model(uint8_t id, uint8_t *data_arr)                                                                            // 2.33
{
    return x10_api_base_.Motor_model(id, data_arr);
}
int8_t X10ApiSerial::Motor_function(uint8_t id, uint8_t index, uint32_t Value)                                                             // 2.34
{
    return x10_api_base_.Motor_function(id, index, Value);
}
int8_t X10ApiSerial::set_Motor_id(uint8_t id, uint8_t newID)                                                                               // 7.0a
{
    return x10_api_base_.set_Motor_id(id, newID);
}
int8_t X10ApiSerial::get_Motor_id(uint8_t id, uint16_t &getID)                                                                             // 7.0b
{
    return x10_api_base_.get_Motor_id(id, getID);
}
