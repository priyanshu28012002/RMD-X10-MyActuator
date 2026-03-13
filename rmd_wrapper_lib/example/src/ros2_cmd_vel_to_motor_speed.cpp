#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "std_msgs/msg/float64_multi_array.hpp"

class CmdVelToWheel : public rclcpp::Node
{
public:
    CmdVelToWheel() : Node("cmdvel_to_wheel")
    {
        cmd_sub_ = this->create_subscription<geometry_msgs::msg::Twist>(
            "/cmd_vel",
            10,
            std::bind(&CmdVelToWheel::cmd_callback, this, std::placeholders::_1));

        wheel_pub_ = this->create_publisher<std_msgs::msg::Float64MultiArray>(
            "/wheel_speed",
            10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50),   // 20 Hz
            std::bind(&CmdVelToWheel::timer_callback, this));

        wheel_radius_ = 0.02;  // 40 mm diameter
        wheel_base_   = 0.05;  // 50 mm separation
    }

private:

    void cmd_callback(const geometry_msgs::msg::Twist::SharedPtr msg)
    {
        linear_  = msg->linear.x;
        angular_ = msg->angular.z;
    }

    void timer_callback()
    {
        double v_left  = linear_ - (angular_ * wheel_base_ / 2.0);
        double v_right = linear_ + (angular_ * wheel_base_ / 2.0);

        double w_left  = v_left  / wheel_radius_;
        double w_right = v_right / wheel_radius_;

        std_msgs::msg::Float64MultiArray msg;
        msg.data = {w_left, w_right};

        wheel_pub_->publish(msg);
    }

    double linear_{0.0};
    double angular_{0.0};

    double wheel_radius_;
    double wheel_base_;

    rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr cmd_sub_;
    rclcpp::Publisher<std_msgs::msg::Float64MultiArray>::SharedPtr wheel_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<CmdVelToWheel>());
    rclcpp::shutdown();
    return 0;
}


/*
Robot Parameters 

Wheel diameter = 40 mm = 0.04 m
Wheel radius = 0.02 m
Wheel separation = 50 mm = 0.05 m

v  = linear velocity (m/s)
w  = angular velocity (rad/s)
L  = wheel separation
R  = wheel radius

2. Differential Drive Equations
Units → rad/s
Wheel linear velocities:
v_left  = v - (w * L / 2)
v_right = v + (w * L / 2)

Convert to wheel angular velocity:
ω_left  = v_left  / R
ω_right = v_right / R

*/