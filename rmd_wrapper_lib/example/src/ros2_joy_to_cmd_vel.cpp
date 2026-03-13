#include "rclcpp/rclcpp.hpp"
#include "sensor_msgs/msg/joy.hpp"
#include "geometry_msgs/msg/twist.hpp"

class JoyToCmd : public rclcpp::Node
{
public:
    JoyToCmd() : Node("joy_to_cmd")
    {
        joy_sub_ = this->create_subscription<sensor_msgs::msg::Joy>(
            "/joy",
            10,
            std::bind(&JoyToCmd::joy_callback, this, std::placeholders::_1));

        cmd_pub_ = this->create_publisher<geometry_msgs::msg::Twist>(
            "/cmd_vel", 10);

        timer_ = this->create_wall_timer(
            std::chrono::milliseconds(50),
            std::bind(&JoyToCmd::timer_callback, this));
    }

private:

    void joy_callback(const sensor_msgs::msg::Joy::SharedPtr msg)
    {
        last_axes_ = msg->axes;
    }

    void timer_callback()
    {
        if(last_axes_.size() < 3)
            return;

        geometry_msgs::msg::Twist cmd;

        cmd.linear.x  = last_axes_[1];   
        cmd.angular.z = last_axes_[2];  

        cmd_pub_->publish(cmd);
    }

    std::vector<float> last_axes_;

    rclcpp::Subscription<sensor_msgs::msg::Joy>::SharedPtr joy_sub_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr cmd_pub_;
    rclcpp::TimerBase::SharedPtr timer_;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    rclcpp::spin(std::make_shared<JoyToCmd>());
    rclcpp::shutdown();
    return 0;
}