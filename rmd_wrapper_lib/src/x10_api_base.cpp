#include <x10_api_base.hpp>

#include "x10_reg.hpp"
#include "crc16.hpp"
#include "SerialComm.hpp"


X10ApiBase::X10ApiBase()
{
    // serial_ = SerialComm();
    send_data_serial_.reserve(RS485_MAX);
    receive_data_serial_.reserve(RS485_MAX);
    std::cout << "Constructor X10ApiBase\n";

}


X10ApiBase::~X10ApiBase()
{
    serial_.serial_shutdown();
    std::cout << "Disconstructor X10ApiBase\n";

}


void X10ApiBase::get_port_address(std::string &port)
{
    serial_.get_port(port);
}


uint16_t X10ApiBase::Crc_check(uint8_t *DATA) 
{

    return crc_.crc16(DATA, RS485_MAX - 2);
}
 
/**
 * @fn					- RS485_send_driver
 * @brief				- send data to the actuator
 * @param[in]			- id of the actuator.
 * @param[in]			- data length
 * @param[in]			- array pointer to store the received data.
 * @return				- None
*/
void X10ApiBase::RS485_send_driver(uint8_t id, uint8_t data_length, uint8_t *p_buffer)
{
    send_data_serial_.clear();
    uint16_t T_Num = 3;
    send_data_serial_.push_back(0x3e);
    send_data_serial_.push_back(id);
    send_data_serial_.push_back(data_length);
    for (T_Num = 3; T_Num < (RS485_MAX - 2); T_Num++)
    {
        send_data_serial_.push_back(p_buffer[T_Num - 3]);
    }

    T_Num = Crc_check(send_data_serial_.data());
    send_data_serial_.push_back(((uint8_t)T_Num));
    send_data_serial_.push_back(((uint8_t)(T_Num >> 8)));

    receive_data_serial_.clear();
    serial_.send_receive_serial_b(send_data_serial_, receive_data_serial_);
}

int8_t X10ApiBase::error_check()
{
    uint16_t check_crc = Crc_check(receive_data_serial_.data());
    uint16_t data_crc = (receive_data_serial_[RS485_MAX - 2] | (receive_data_serial_[RS485_MAX - 1] << 8));

    if ((check_crc == data_crc) && (send_data_serial_[1] == receive_data_serial_[1]))
        return 0;

    return 1;
}

/***************************Serial communication function**************************/

/**
 * @fn					- rmdX10_init
 * @brief				- Inititalize the communication between the actuator and the controller.
 * @param[in]			- None
 * @return				- None
 */
void X10ApiBase::rmdX10_init()
{
    serial_.serial_init();
}

/**
 * @fn					- rmdX10_shut_down
 * @brief				- stop the communication between the actuator and the controller.
 * @param[in]			- None
 * @return				- None
 */
void X10ApiBase::rmdX10_shut_down()
{
    serial_.serial_shutdown();
}

//*********************************Function calls*****************************************/

/**
 * @fn					- Motor_read_pid
 * @brief				- read the parameters of current, speed, position loop, KP and KI at one time, and the data type is uint8_t.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data.
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Motor_read_pid(uint8_t id, uint8_t *data_arr)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_read_pid);

    data_arr[0] = (uint8_t)(receive_data_serial_[5]);  // currKP
    data_arr[1] = (uint8_t)(receive_data_serial_[6]);  // currKI
    data_arr[2] = (uint8_t)(receive_data_serial_[7]);  // spdKP
    data_arr[3] = (uint8_t)(receive_data_serial_[8]);  // spdKI
    data_arr[4] = (uint8_t)(receive_data_serial_[9]);  // posKP
    data_arr[5] = (uint8_t)(receive_data_serial_[10]); // posKI

    return error_check();
}

/**
 * @fn					- Write_pid_RAM
 * @brief				- write the parameters of current, speed, position loop KP and KI to RAM at one time, and it will **not** be saved after power off.
 * @param[in]			- id of the actuator.
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Write_pid_RAM(uint8_t id, uint8_t CurrKP, uint8_t CurrKI, uint8_t SpdKP, uint8_t SpdKI, uint8_t PosKP, uint8_t PosKI)
{
    x10_reg_.a_Write_pid_RAM[2] = (uint8_t)CurrKP;
    x10_reg_.a_Write_pid_RAM[3] = (uint8_t)CurrKI;
    x10_reg_.a_Write_pid_RAM[4] = (uint8_t)SpdKP;
    x10_reg_.a_Write_pid_RAM[5] = (uint8_t)SpdKI;
    x10_reg_.a_Write_pid_RAM[6] = (uint8_t)PosKP;
    x10_reg_.a_Write_pid_RAM[7] = (uint8_t)PosKI;

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Write_pid_RAM);
    return error_check();
}

/**
 * @fn					- Write_pid_ROM
 * @brief				- write the parameters of current, speed, position loop KP and KI to ROM at one time, which can be saved after power off.
 * @param[in]			- id of the actuator.
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Write_pid_ROM(uint8_t id, uint8_t CurrKP, uint8_t CurrKI, uint8_t SpdKP, uint8_t SpdKI, uint8_t PosKP, uint8_t PosKI)
{
    x10_reg_.a_Write_pid_ROM[2] = (uint8_t)CurrKP;
    x10_reg_.a_Write_pid_ROM[3] = (uint8_t)CurrKI;
    x10_reg_.a_Write_pid_ROM[4] = (uint8_t)SpdKP;
    x10_reg_.a_Write_pid_ROM[5] = (uint8_t)SpdKI;
    x10_reg_.a_Write_pid_ROM[6] = (uint8_t)PosKP;
    x10_reg_.a_Write_pid_ROM[7] = (uint8_t)PosKI;

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Write_pid_ROM);
    return error_check();
}

/**
 * @fn					- Motor_read_accel
 * @brief				- read the acceleration parameters of the current actuator.
 * @param[in]			- id of the actuator.
 * @return				- acceleration value of the actuator
 */
int8_t X10ApiBase::Motor_read_accel(uint8_t id, int32_t &get_accel)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Read_Accel);

    get_accel = (int32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}


int8_t X10ApiBase::Write_accel_ALL(uint8_t id, uint8_t index, uint32_t accel) { // 2.5

    x10_reg_.a_Write_Accel[1] = (uint8_t)index;
    x10_reg_.a_Write_Accel[4] = (uint8_t)accel;
    x10_reg_.a_Write_Accel[5] = (uint8_t)(accel >> 8);
    x10_reg_.a_Write_Accel[6] = (uint8_t)(accel >> 16);
    x10_reg_.a_Write_Accel[7] = (uint8_t)(accel >> 24);


    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Write_Accel);
    return error_check();
}

/**
 * @fn					- multi_turn_current_pos
 * @brief				- read the multi-turn position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @return				- error status
 */
int8_t X10ApiBase::multi_turn_current_pos(uint8_t id, int32_t &get_current_enc_pose)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_multi_turn_current_pos);

    get_current_enc_pose = (int32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}

/**
 * @fn					- multi_turn_original_pos
 * @brief				- read the multi-turn original position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data.
 * @return				- error status
*/
int8_t X10ApiBase::multi_turn_original_pos(uint8_t id, int32_t &get_original_enc_pose)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_multi_turn_original_pos);
    get_original_enc_pose = (int32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}


/**
 * @fn					- multi_turn_zero_off_pos
 * @brief				- read the multi-turn zero offset position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data.
 * @return				- error status
*/
int8_t X10ApiBase::multi_turn_zero_off_pos(uint8_t id, int32_t get_zero_off_pose)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_multi_turn_zero_off_pos);
    get_zero_off_pose = (int32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}

/**
 * @fn					- set_multi_turn_zero
 * @brief				- set the multi-turn zero offset position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data.
 * @return				- error status
 * @note                - restart motor after this command
*/
int8_t X10ApiBase::set_multi_turn_zero(uint8_t id, int32_t offset)
{

    x10_reg_.a_Write_code_zero_ROM[4] = (uint8_t)offset;
    x10_reg_.a_Write_code_zero_ROM[5] = (uint8_t)(offset >> 8);
    x10_reg_.a_Write_code_zero_ROM[6] = (uint8_t)(offset >> 16);
    x10_reg_.a_Write_code_zero_ROM[7] = (uint8_t)(offset >> 24);
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Write_code_zero_ROM);
    return error_check();
}

//restart motor after this command
/**
 * @fn					- set_current_turn_zero
 * @brief				- set the current turn zero offset position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @return				- error status
*/
int8_t X10ApiBase::set_current_turn_zero(uint8_t id) // 2.10
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Write_NOWcode_zero_ROM);
    return error_check();
}

/**
 * @fn					- get_single_enc
 * @brief				- read the single-turn position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data.
 * @return				- error status
*/
int8_t X10ApiBase::get_single_enc(uint8_t id, int16_t *data_arr) // 2.11
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_enc_single_turn);
    data_arr[0] = (int16_t)((receive_data_serial_[5]) | (receive_data_serial_[6] << 8));  // encoder value
    data_arr[1] = (int16_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8));  // encoder raw
    data_arr[2] = (int16_t)((receive_data_serial_[9]) | (receive_data_serial_[10] << 8)); // encoder offset

    return error_check();
}

/**
 * @fn					- get_multi_angle
 * @brief				- read the multi-turn position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @param[in]			- motorAngle
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::get_multi_angle(uint8_t id, int32_t motorAngle) // 2.12
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_motorAngle_many);
    motorAngle = (int32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}

/**
 * @fn					- get_single_angle
 * @brief				- read the single-turn position of the encoder, which represents the rotation angle of the actuator output shaft, including the multi-turn angle.
 * @param[in]			- id of the actuator.
 * @param[in]			- motorAngle
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::get_single_angle(uint8_t id, int16_t motorAngle) // 2.13
{

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_motorAngle_single);
    motorAngle = (int16_t)((receive_data_serial_[9]) | (receive_data_serial_[10] << 8));
    return error_check();
}

/**
 * @fn					- Motor_state1
 * @brief				- reads the current actuator temperature, voltage and error status flags.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data.
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Motor_state1(uint8_t id, int16_t *data_arr)
{

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_state1);

    data_arr[0] = (int8_t)receive_data_serial_[4];                                         // temp
    data_arr[1] = (uint8_t)receive_data_serial_[6];                                        // brake release command
    data_arr[2] = (uint16_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8));  // voltage
    data_arr[3] = (uint16_t)((receive_data_serial_[9]) | (receive_data_serial_[10] << 8)); // error state
    return error_check();
}

/**
 * @fn					- Motor_state2
 * @brief				- reads the temperature, speed and encoder position of the current actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Motor_state2(uint8_t id, int16_t *data_arr)
{

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_state2);

    data_arr[0] = (int8_t)(receive_data_serial_[4]);                                      // temp
    data_arr[1] = (int16_t)((receive_data_serial_[5]) | (receive_data_serial_[6] << 8));  // torque current
    data_arr[2] = (int16_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8));  // motor speed
    data_arr[3] = (int16_t)((receive_data_serial_[9]) | (receive_data_serial_[10] << 8)); // motor angle
    return error_check();
}

/**
 * @fn					- Motor_state3
 * @brief				- reads the current actuator temperature and phase current data.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Motor_state3(uint8_t id, int16_t *data_arr)
{

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_state3);
    data_arr[0] = (int8_t)receive_data_serial_[4];                                        // temp
    data_arr[1] = (int16_t)((receive_data_serial_[5]) | (receive_data_serial_[6] << 8));  // Phase A current
    data_arr[2] = (int16_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8));  // Phase B current
    data_arr[3] = (int16_t)((receive_data_serial_[9]) | (receive_data_serial_[10] << 8)); // Phase C current
    
    return error_check();
}

/**
 * @fn					- Motor_shut_down
 * @brief				- Turns off the actuator output and also clears the actuator running state, not in any closed loop mode.
 * @param[in]			- id of the actuator.
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Motor_shut_down(uint8_t id)
{

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_shut_down);
    return error_check();
}

/**
 * @fn					- Motor_stop
 * @brief				- Stop the actuator, the closed-loop mode where the actuator is still running, just stop the actuator speed.
 * @param[in]			- id of the actuator.
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::Motor_stop(uint8_t id)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_stop);
    return error_check();
}

/**
 * @fn					- torqueControl
 * @brief				- control the torque of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- torque of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::torqueControl(uint8_t id, int32_t iqControl)
{
    x10_reg_.a_Motor_torque[4] = (uint8_t)iqControl;
    x10_reg_.a_Motor_torque[5] = (uint8_t)(iqControl >> 8);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_torque);
    return error_check();
}

/**
 * @fn					- speedControl
 * @brief				- control the speed of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- speed of the actuator
 * @return				- error status (0: no_error | 1: error )
 */
int8_t X10ApiBase::speedControl(uint8_t id, int32_t speed)
{
    x10_reg_.a_speedControl[4] = (uint8_t)speed;
    x10_reg_.a_speedControl[5] = (uint8_t)(speed >> 8);
    x10_reg_.a_speedControl[6] = (uint8_t)(speed >> 16);
    x10_reg_.a_speedControl[7] = (uint8_t)(speed >> 24);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_speedControl);
    return error_check();
}

/**
 * @fn					- speedControl
 * @brief				- control the speed of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- speed of the actuator
 * @param[in]			- get_speed of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::speedControl(uint8_t id, int32_t speed, int16_t &get_speed)
{
    x10_reg_.a_speedControl[4] = (uint8_t)speed;
    x10_reg_.a_speedControl[5] = (uint8_t)(speed >> 8);
    x10_reg_.a_speedControl[6] = (uint8_t)(speed >> 16);
    x10_reg_.a_speedControl[7] = (uint8_t)(speed >> 24);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_speedControl);

    get_speed = (int16_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8));
    return error_check();
}

/**
 * @fn					- abs_pose_control
 * @brief				- control the absolute position of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- maxSpeed of the actuator
 * @param[in]			- angle of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::abs_pose_control(uint8_t id, uint16_t maxSpeed, int32_t angle)
{
    x10_reg_.a_abs_pose_control[2] = (uint8_t)(maxSpeed);
    x10_reg_.a_abs_pose_control[3] = (uint8_t)(maxSpeed >> 8);
    x10_reg_.a_abs_pose_control[4] = (uint8_t)(angle);
    x10_reg_.a_abs_pose_control[5] = (uint8_t)(angle >> 8);
    x10_reg_.a_abs_pose_control[6] = (uint8_t)(angle >> 16);
    x10_reg_.a_abs_pose_control[7] = (uint8_t)(angle >> 24);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_abs_pose_control);
    return error_check();
}

/**
 * @fn					- single_turn
 * @brief				- control the single-turn position of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- spinDir of the actuator
 * @param[in]			- maxSpeed of the actuator
 * @param[in]			- angle of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::single_turn(uint8_t id, uint8_t spinDir, uint16_t maxSpeed, uint16_t angle)
{
    x10_reg_.a_single_turn[1] = (uint8_t)(spinDir);
    x10_reg_.a_single_turn[2] = (uint8_t)(maxSpeed);
    x10_reg_.a_single_turn[3] = (uint8_t)(maxSpeed >> 8);
    x10_reg_.a_single_turn[4] = (uint8_t)(angle);
    x10_reg_.a_single_turn[5] = (uint8_t)(angle >> 8);
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_single_turn);

    return error_check();
}

/**
 * @fn					- increment_control
 * @brief				- control the increment position of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- maxSpeed of the actuator
 * @param[in]			- angle of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::increment_control(uint8_t id, uint16_t maxSpeed, int32_t angle) // 2.23
{
    x10_reg_.a_Motor_add[2] = (uint8_t)(maxSpeed);
    x10_reg_.a_Motor_add[3] = (uint8_t)(maxSpeed >> 8);
    x10_reg_.a_Motor_add[4] = (uint8_t)(angle);
    x10_reg_.a_Motor_add[5] = (uint8_t)(angle >> 8);
    x10_reg_.a_Motor_add[6] = (uint8_t)(angle >> 16);
    x10_reg_.a_Motor_add[7] = (uint8_t)(angle >> 24);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_add);
    return error_check();
}

/**
 * @fn					- Motor_mode
 * @brief				- read the mode of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- mode of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::Motor_mode(uint8_t id, uint8_t &mode)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_mode);
    mode = receive_data_serial_[10];
    return error_check();
}

/**
 * @fn					- Motor_mode
 * @brief				- set the mode of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- mode of the actuator
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::get_motor_power(uint8_t id) // 2.25
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_power);
    return error_check();
}

/**
 * @fn					- Motor_reset
 * @brief				- Reset the actuator.
 * @param[in]			- id of the actuator.
 * @return				- None
 */
void X10ApiBase::Motor_reset(uint8_t id)
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_reset);
}


/**
 * @fn					- Motor_brake_release
 * @brief				- release the brake of the actuator.
 * @param[in]			- id of the actuator.
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::Motor_brake_release(uint8_t id)                                                                                       // 2.27

{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_OPEN_RlyCtrlRslt);
    return error_check();
}

/**
 * @fn					- Motor_brake_lock
 * @brief				- lock the brake of the actuator.
 * @param[in]			- id of the actuator.
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::Motor_brake_lock(uint8_t id)                                                                                       // 2.28

{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_CLOSE_RlyCtrlRslt);
    return error_check();
}

/**
 * @fn					- Motor_runtime
 * @brief				- read the running time of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
*/
int8_t X10ApiBase::Motor_runtime(uint8_t id, uint32_t &runTime) {                                                                        // 2.29

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_time);

    runTime = (uint32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}

/**
 * @fn					- Motor_edition
 * @brief				- read the edition of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
*/
int8_t X10ApiBase::Motor_edition(uint8_t id, uint32_t &sysDate) {                                                         // 2.30


    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_edition);

    sysDate = (uint32_t)((receive_data_serial_[7]) | (receive_data_serial_[8] << 8) | (receive_data_serial_[9] << 16) | (receive_data_serial_[10] << 24));
    return error_check();
}

/**
 * @fn					- Motor_comm_protect
 * @brief				- set the communication protection time of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- communication protection time of the actuator.
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::Motor_comm_protect(uint8_t id, uint32_t commProtect)                                                                  // 2.31
{
    x10_reg_.a_Motor_comm_time[4] = (uint8_t)commProtect;
    x10_reg_.a_Motor_comm_time[5] = (uint8_t)(commProtect >> 8);
    x10_reg_.a_Motor_comm_time[6] = (uint8_t)(commProtect >> 16);
    x10_reg_.a_Motor_comm_time[7] = (uint8_t)(commProtect >> 24);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_comm_time);
    return error_check();
}


/**
 * @fn					- Motor_baudrate
 * @brief				- set the baudrate of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- baudrate of the actuator.
 * @return				- error status (0: no_error | 1: error )
*/
void X10ApiBase::Motor_baudrate(uint8_t id, uint8_t baudrate)                                                                  // 2.32
{
    x10_reg_.a_Motor_baudrate[4] = (uint8_t)baudrate;
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_baudrate);

}

/**
 * @fn					- Motor_model
 * @brief				- read the model number of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::Motor_model(uint8_t id, uint8_t *data_arr)                                                                                        //2.33
{
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_model);
    data_arr[0] = (uint8_t)receive_data_serial_[4];
    data_arr[1] = (uint8_t)receive_data_serial_[5];
    data_arr[2] = (uint8_t)receive_data_serial_[6];
    data_arr[3] = (uint8_t)receive_data_serial_[7];
    data_arr[4] = (uint8_t)receive_data_serial_[8];
    data_arr[5] = (uint8_t)receive_data_serial_[9];
    data_arr[6] = (uint8_t)receive_data_serial_[10];

    return error_check();
}


/**
 * @fn					- Motor_function
 * @brief				- read the function of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::Motor_function(uint8_t id, uint8_t index, uint32_t Value)                                                                         //2.34
{
    x10_reg_.a_Motor_function[1] = (uint8_t)index;
    x10_reg_.a_Motor_function[4] = (uint8_t)Value;
    x10_reg_.a_Motor_function[5] = (uint8_t)(Value >> 8);
    x10_reg_.a_Motor_function[6] = (uint8_t)(Value >> 16);
    x10_reg_.a_Motor_function[7] = (uint8_t)(Value >> 24);

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_function);
    return error_check();
}


/**
 * @fn					- Motor_id
 * @brief				- set the id of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- new id of the actuator.
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::set_Motor_id(uint8_t id, uint8_t newID)                                                        // 7.0
{

    x10_reg_.a_Motor_RS485ID[1] = 0;
    x10_reg_.a_Motor_RS485ID[7] = (uint8_t)newID;

    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_RS485ID);
    return error_check();
}

/**
 * @fn					- Motor_id
 * @brief				- read the id of the actuator.
 * @param[in]			- id of the actuator.
 * @param[in]			- array pointer to store the received data
 * @return				- error status (0: no_error | 1: error )
*/
int8_t X10ApiBase::get_Motor_id(uint8_t id, uint16_t &getID)
{
    x10_reg_.a_Motor_RS485ID[1] = 1;
    x10_reg_.a_Motor_RS485ID[7] = 0;
    RS485_send_driver(id, DATA_MAX, x10_reg_.a_Motor_RS485ID);
    return error_check();
}
