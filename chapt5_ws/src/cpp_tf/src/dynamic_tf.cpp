#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include <chrono>
using namespace std::chrono_literals;
//动态TF发布和静态的代码区别是使用定时器不断广播
class DynamicTfBroadcaster : public rclcpp::Node
{
public:
    DynamicTfBroadcaster():Node("dynamic_tf_broadcaster")
    {
        m_static_broadcaster = std::make_shared<tf2_ros::TransformBroadcaster>(this);
        mp_timer=this->create_wall_timer(100ms, [this]() {
            this->pub_static_tf();
        });
    }

    void pub_static_tf()
    {
        geometry_msgs::msg::TransformStamped transform;
        transform.header.stamp = this->now();
        transform.header.frame_id = "map";
        transform.child_frame_id = "base_link";
        transform.transform.translation.x = 2.0;
        transform.transform.translation.y = 3.0;
        transform.transform.translation.z = 0.0;
        tf2::Quaternion q;
        q.setRPY(0.0, 0.0, 30.0 * M_PI / 180.0);
        transform.transform.rotation = tf2::toMsg(q);
        m_static_broadcaster->sendTransform(transform);
    }
private:
    std::shared_ptr<tf2_ros::TransformBroadcaster> m_static_broadcaster;
    //创建定时器
    rclcpp::TimerBase::SharedPtr mp_timer;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<DynamicTfBroadcaster>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}