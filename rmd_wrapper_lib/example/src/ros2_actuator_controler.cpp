#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"
#include "x10_api.hpp"


class MYACTUATOR : public rclcpp::Node
{
public:
    MYACTUATOR() : Node("cmdvel_to_wheel")
    {
        // Motor API init
        xobj = new X10ApiSerial();

        std::string port = "/dev/ttyUSB0";
        xobj->get_port_address(port);
        xobj->rmdX10_init();

        // ROS2 subscriber
        wheel_speed_sub_ = this->create_subscription<std_msgs::msg::Float64MultiArray>(
            "/wheel_speed",
            10,
            std::bind(&MYACTUATOR::cmd_callback, this, std::placeholders::_1));

        motor_command_timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50), // 20 Hz
            std::bind(&MYACTUATOR::motor_command_, this));
    }

    ~MYACTUATOR()
    {
        Motor_stop(xobj, 1);
        delete xobj;
    }

private:
    void cmd_callback(const std_msgs::msg::Float64MultiArray::SharedPtr msg)
    {
        std::string data_str;

        for (size_t i = 0; i < msg->data.size(); i++)
        {
            data_str += std::to_string(msg->data[i]) + " ";
        }

        lMoterSpeed = msg->data[0];
        rMoterSpeed = msg->data[1];
        // RCLCPP_INFO(this->get_logger(), "Received wheel speed data: [%s]", data_str.c_str());
    }

    void motor_command_()
    {
        if (abs(abs(lastLeftMoterSpeed) - abs(lMoterSpeed)) > 3)
        {
            lMoterSpeed = lMoterSpeed;
            RCLCPP_INFO(this->get_logger(), "motor_command_ speedControl left: [%s]", std::to_string(lMoterSpeed).c_str());
            speedControl(xobj, lMoterId, lMoterSpeed * 500);
            lastLeftMoterSpeed = lMoterSpeed;
        }
        else
        {
            // RCLCPP_INFO(this->get_logger(), "Skip the Motor Command");
        }
    }
    void Motor_stop(X10ApiSerial *xobj, uint8_t id)
    {

        int8_t x = xobj->Motor_stop(id);
        std::cout << "Motor_stop Result" << x << std::endl;
    }

    void speedControl(X10ApiSerial *xobj, uint8_t id, int speed)
    {

        int8_t x = xobj->speedControl(id, speed);
        std::cout << "speedControl Result " << (int)x << std::endl;
    }

    rclcpp::TimerBase::SharedPtr motor_command_timer_;

    // Motor IDs
    int lMoterId = 1;
    int rMoterId = 2;
    int lMoterSpeed = 0;
    int rMoterSpeed = 0;
    int lastLeftMoterSpeed = 0;

    // Motor API object
    X10ApiSerial *xobj;
    // ROS2 variable
    rclcpp::Subscription<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_speed_sub_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<MYACTUATOR>());
    rclcpp::shutdown();
    return 0;
}