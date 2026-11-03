#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/static_transform_broadcaster.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"

class StaticTfBroadcaster : public rclcpp::Node
{
public:
    StaticTfBroadcaster():Node("static_tf_broadcaster")
    {
        m_static_broadcaster = std::make_shared<tf2_ros::StaticTransformBroadcaster>(this);

    }

    void pub_static_tf()
    {
        geometry_msgs::msg::TransformStamped transform;
        transform.header.stamp = this->now();
        transform.header.frame_id = "map";
        transform.child_frame_id = "target_point";
        transform.transform.translation.x = 5.0;
        transform.transform.translation.y = 3.0;
        transform.transform.translation.z = 0.0;
        tf2::Quaternion q;
        q.setRPY(0.0, 0.0, 60.0 * M_PI / 180.0);
        transform.transform.rotation = tf2::toMsg(q);
        m_static_broadcaster->sendTransform(transform);
    }
private:
    std::shared_ptr<tf2_ros::StaticTransformBroadcaster> m_static_broadcaster;
};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<StaticTfBroadcaster>();
    node->pub_static_tf();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}