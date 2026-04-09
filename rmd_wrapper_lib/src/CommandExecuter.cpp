#include "CommandExecuter.hpp"

CommandExecuter::CommandExecuter(X10ApiSerial *driver,
                                 CommandScheduler *scheduler, MotorManager *motorManager)
    : driver_(driver),
      scheduler_(scheduler),
      motorManager_(motorManager),
      running_(false)
{
    driver_->rmdX10_init();
}

CommandExecuter::~CommandExecuter()
{
    stopExecution();
}

void CommandExecuter::startExecution()
{
    running_ = true;
    worker_ = std::thread(&CommandExecuter::processLoop, this);
}

void CommandExecuter::stopExecution()
{
    running_ = false;

    if (worker_.joinable())
        worker_.join();
}

void CommandExecuter::processLoop()
{
    while (running_)
    {
        MotorCommand cmd;

        if (!scheduler_->getNextCommand(cmd))
            continue;

        executeCommand(cmd);
    }
}

void CommandExecuter::parseMoterState1(int16_t* buffer)
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

void CommandExecuter::parseMoterState2(int16_t* buffer)
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

void CommandExecuter::parseMoterState3(int16_t* buffer)
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

void CommandExecuter::executeCommand(MotorCommand cmd)
{

    if (cmd.type == CommandType::SPEED)
    {
        // std::cout << "SPEED Command" << std::endl;

        driver_->speedControl(cmd.motorId, cmd.speed);
    }

    if (cmd.type == CommandType::POSITION)
    {
        std::cout << "POSITION Command" << std::endl;

        //  driver_->Motor_angle_control(cmd.motorId,cmd.angle,cmd.speed);
    }

    if (cmd.type == CommandType::STATUS)
    {

        int16_t buffer1[16] = {0};
        int16_t buffer2[16] = {0};
        int16_t buffer3[16] = {0};

        int8_t x = driver_->Motor_state1(cmd.motorId, buffer1); // 2.14
        int8_t y = driver_->Motor_state2(cmd.motorId, buffer2); // 2.15
        int8_t z = driver_->Motor_state3(cmd.motorId, buffer3);
        // std::cout << "Motor_state Result " << (int)x << (int)y << (int)z << std::endl;

        // int temperature;
        // int voltage;
        // int current;
        // int speed;
        // int position;

        // int targetSpeed;
        // int torque;

        // int errorCode;
        // int faultFlags;

        // temperature = buffer1[2];
        // voltage = buffer1[3];
        // current = buffer1[4];
        // speed = buffer1[5];
        // targetSpeed = buffer2[1];
        // torque = buffer2[3];

        // errorCode = buffer3[7];
        // faultFlags = buffer3[0];

        parseMoterState1(buffer1);
                parseMoterState2(buffer2);
        parseMoterState3(buffer3);


        // motorManager_->setTemp(cmd.motorId, temperature);
        // motorManager_->setVoltage(cmd.motorId, voltage);
        // motorManager_->setCurrent(cmd.motorId, current);
        // motorManager_->setSpeed(cmd.motorId, speed);
        // motorManager_->setPosition(cmd.motorId, position);
        // motorManager_->setTargetSpeed(cmd.motorId, targetSpeed);
        // motorManager_->setTorque(cmd.motorId, torque);

        // std::cout << "STATUS Command" << std::endl;

        // driver_->Motor_speed_control(cmd.motorId, cmd.speed);
    }

    if (cmd.type == CommandType::EMERGENCY)
    {
        std::cout << "EMERGENCY Command" << std::endl;

        //  driver_->Motor_angle_control(cmd.motorId,cmd.angle,cmd.speed);
    }

    if (cmd.type == CommandType::STOP)
    {
        driver_->Motor_stop(cmd.motorId);
        // std::cout << "Stop Command" << std::endl;

        //  driver_->Motor_angle_control(cmd.motorId,cmd.angle,cmd.speed);
    }
}
