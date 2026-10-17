#include "turtlenode_status/msg/turtle_node_status.hpp"
#include "turtlesim/msg/pose.hpp"
#include "rclcpp/rclcpp.hpp"

class Turtle_Status_Pub : public rclcpp::Node{
private:
//创建订阅小海龟状态的共享指针
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr get_v;
//创建发布小海龟状态的共享指针
    rclcpp::Publisher<turtlenode_status::msg::TurtleNodeStatus>::SharedPtr show_status;
//记录小海龟状态数据
    double m_now_speed=0.0;
    double m_lasttime_speed=0.0;
//实例化自定义消息接口话题
    turtlenode_status::msg::TurtleNodeStatus m_status_msg;
public:
    Turtle_Status_Pub():Node("Turtle_status"){
        //初始化发布者
        show_status=this->create_publisher<turtlenode_status::msg::TurtleNodeStatus>("Turtle_status",10);
        //通过初始化订阅拿到小海龟Posemsg,并读取到对象
        get_v=this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose",10,[this](const turtlesim::msg::Pose::SharedPtr msg)->void{
            m_now_speed=msg->linear_velocity;
            //RCLCPP_INFO(get_logger(),"now_speed is %f",m_now_speed);
            //逻辑：当当前线速度为零且与上一时刻速度不同时停下
            if(m_now_speed==0.0&&m_lasttime_speed!=m_now_speed){
                m_status_msg.stamp=this->now();
                m_status_msg.running_status=false;
                show_status->publish(m_status_msg); 
            }else if(m_now_speed!=0.0&&m_lasttime_speed==0){
                m_status_msg.stamp=this->now();
                m_status_msg.running_status=true;  
                show_status->publish(m_status_msg);              
            }
            //更新上一次时刻状态
            m_lasttime_speed=m_now_speed;
        });
        m_status_msg.robot_name=get_v->get_topic_name();
    }   

};

int main(int argc,char**argv){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<Turtle_Status_Pub>();
    rclcpp::spin(node);
    rclcpp::shutdown();

    return 0;
}