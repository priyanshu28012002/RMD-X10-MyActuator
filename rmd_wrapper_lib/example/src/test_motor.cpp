#include "MyActuator.hpp"

int main()
{
    MyActuator robot(2, "/dev/ttyUSB0");

    robot.motor(1)->setSpeed(200);

    // robot.motor(2)->moveToAngle(90,100);

    for(int i=0; i<10;i++){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

/*

//init
MyActuator robot(3, "/dev/ttyUSB0");
robot.motor(1)->inti();
robot.motor(2)->inti();
robot.motor(3)->inti();
robot.motor()->inti(); // For all;

//Emergency
robot.motor(1)->stop()
robot.motor(1)->applyBreak()
robot.motor(1)->EmergencyStop() // deinit,applybreak,stop

//Status
robot.motor(1)->getTemp()
robot.motor(1)->getVoltage()
robot.motor(1)->getCurrent()
robot.motor(1)->getPosition()
robot.motor(1)->getAngle()

//Command
robot.motor(2)->moveToAngle(90,100);
robot.motor(2)->moveToPosition(90,100);

//Setting
robot.motor(1)->setAcc(20)
robot.motor(1)->setPid()
robot.motor(1)->setTourqe()

*/