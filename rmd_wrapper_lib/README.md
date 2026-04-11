# RMD-X10-MyActuator

Application   ->
MyActuator ->
MotorManager-> Motor -> CommandScheduler (Priority Queue + Thread) -> X10ApiSerial -> RS485 Bus ->
Motor Driver


// singalton

1. Make the constructor using the     
constexpr int NUM_MOTORS = 2;
constexpr const char *MOTOR_PORT = "/dev/ttyUSB0";
MyActuator actuator(NUM_MOTORS, MOTOR_PORT);

If the motor is connected Successfull the State will move from the NONE - > ideal state

2. actuator.init()
driver_->rmdX10_init();
It init the port and the driver if success full we move to the  ideal state to READY 

2.1. actuator.resetMotor(int motorID) 
Reset the all motor 
Make the current speed of the motor zero 
Set the Default MaxSpeed MinSpeed
 

3. actuator.setMode()
Motor Can only oprate in the ready state 
We have 4 mode of operation 
NONE,SPEED,ANGLE,SETTING

3.1 Setting Mode
First we Will Move to the Setting Stage 
Set the new Max Min Speed

3.2 SPEED Mode
We can Define each Motor speed Individually Using the Joystick asix 
3 for the Forword and Backword 
0 for the left and write 
 
3.3 Angle Mode
We can Define each Motor Angle Individually They have to move We will can calculate the angle using the Angle and Wheel Dia
we can make the 4 Feature Move forword , Move backword , turn Left turn Right


4 updateStatus
MotorState.hpp
struct MotorState
{

    double temperature = 0.0;
    double voltage = 0.0;
    double current = 0.0;
    double speed = 0.0;
    double position = 0.0;

    double targetSpeed = 0.0;
    double torque = 0.0;

    double errorCode = 0.0;
    double faultFlags = 0.0;
};

bool updateStatus();
void parseMoterState1(int16_t *buffer);
void parseMoterState2(int16_t *buffer);
void parseMoterState3(int16_t *buffer);

5. actuator.fini();

driver_->rmdX10_shut_down();
State NONE
MODE NONE
