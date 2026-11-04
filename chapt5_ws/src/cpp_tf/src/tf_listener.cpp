#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2_ros/transform_listener.h"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2_geometry_msgs/tf2_geometry_msgs.hpp"
#include "tf2_ros/buffer.h"
#include "tf2/utils.h"
#include <chrono>
using namespace std::chrono_literals;
//继续完成TF监听器的代码，主要是创建一个节点，订阅TF变换，并在定时器回调中处理接收到的变换数据
class TfListener : public rclcpp::Node
{
public:

    void get_transform();

    TfListener():Node("tf_listener")
    {
        mp_buffer = std::make_shared<tf2_ros::Buffer>(this->get_clock());
        mp_listener = std::make_shared<tf2_ros::TransformListener>(*mp_buffer, this);
        // 创建TF监听器
        mp_timer=this->create_wall_timer(3s, [this]() {
            this->get_transform();
        });
    }

private:
    //创建定时器
    rclcpp::TimerBase::SharedPtr mp_timer;
    std::shared_ptr<tf2_ros::Buffer> mp_buffer;
    std::shared_ptr<tf2_ros::TransformListener> mp_listener;

};

void TfListener::get_transform()
{
    geometry_msgs::msg::TransformStamped transform;
    try {
        //查询坐标关系
        const auto transform = mp_buffer->lookupTransform("base_link", "target_point", this->get_clock()->now(),rclcpp::Duration::from_seconds(1.0));
        //获取查询结果
        auto translation = transform.transform.translation;
        auto rotation = transform.transform.rotation;
        double roll, pitch, yaw;
        tf2::getEulerYPR(rotation, yaw, pitch, roll);
        RCLCPP_INFO(this->get_logger(),"平移:%f, %f, %f", translation.x, translation.y, translation.z);
        RCLCPP_INFO(this->get_logger(),"旋转:%f, %f, %f", yaw, pitch, roll);

    } catch (const std::exception&e) {
        RCLCPP_WARN(this->get_logger(), "Could not get transform: %s", e.what());
    }
}

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TfListener>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}