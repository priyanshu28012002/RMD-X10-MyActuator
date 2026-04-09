MyActuator api

struct MyActuatorState{

IDEAL
READY
RUNNING
}

struct MotorStatus
{
    double temperature = 0;
    double voltage = 0;
    double current = 0;
    double speed = 0;
    double position = 0;
    double targetSpeed = 0;
    double torque = 0;
};

MyActuator actuator(NUM_MOTORS, MOTOR_PORT);
actuator.init(); IDEAL -> READY
actuator.validId(int id);
actuator.start(); READY -> RUNNING
actuator.isRunning();
actuator.getState
actuator.getSatus(); voltage current temp angle position

actuator.incrementAngle(1 motor id ,360 angle );
actuator.incrementAngleAll(360 angle );

actuator.setRMP(1 motor id ,2000 RPM);
actuator.setRMPAll(1 motor id ,2000 RPM);

actuator.beakLock();
actuator.beakRelease();

actuator.stop();
actuator.shutdown();
actuator.reset();

actuator.emergencyStop();
actuator.stop();  RUNNING -> READY
actuator.dinit(); READY -> IDEAL

