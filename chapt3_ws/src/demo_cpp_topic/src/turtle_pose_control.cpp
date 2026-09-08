//该程序用于对小海龟位置实现监听和闭环控制，根据随机生成目的地控制小海龟移动
#include"rclcpp/rclcpp.hpp"
//小海龟控制消息接口
#include"geometry_msgs/msg/twist.hpp"
//小海龟位姿消息接口
#include"turtlesim/msg/pose.hpp"
#include<chrono>
#include<cmath>
#include<random>
const double MIN_SIDE = 1.0;
const double MAX_SIDE = 10.0;
const double KP_LINEAR  = 1.5;        // 线速度比例增益
const double KP_ANGULAR = 4.0;        // 角速度比例增益
using  namespace std::chrono_literals;

class TurtlePoseControl : public rclcpp::Node{

    private:
    double goal_x=5.5,goal_y=5.5;
    double current_x=5.5,current_y=5.5;
    double current_theta=0.0;
    //通信与定时器指针
    rclcpp::Subscription<turtlesim::msg::Pose>::SharedPtr m_turtle_pose;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr m_turtle_control;
    rclcpp::TimerBase::SharedPtr m_control_timer;
    rclcpp::TimerBase::SharedPtr m_rest_timer;
    //声明随机数参量
    std::mt19937 m_gen;
    std::uniform_real_distribution<double> m_distrib;

public:
    //构造函数初始化节点，初始化订阅者和发布者，初始化随机数
    TurtlePoseControl():Node("turtle_pose_control"),m_gen(std::random_device{}()),m_distrib(MIN_SIDE, MAX_SIDE){         
        m_turtle_pose=this->create_subscription<turtlesim::msg::Pose>("/turtle1/pose",10,[this](const turtlesim::msg::Pose::SharedPtr msg)->void{
            //将位置话题消息读取到类内
            current_x=msg->x;
            current_y=msg->y;
            current_theta=msg->theta;
            //RCLCPP_INFO(get_logger(),"Now turtle1 is on (%f,%f,%f)",current_x,current_y,current_theta);

        });
        m_turtle_control=this->create_publisher<geometry_msgs::msg::Twist>("/turtle1/cmd_vel",10); 
        //生成初始随机点
        random_point_output();
        //用定时器向小海龟控制节点发布话题
        m_control_timer=this->create_wall_timer(20ms,std::bind(&TurtlePoseControl::ctl_func));
        m_rest_timer=this->create_wall_timer(2s,std::bind(&TurtlePoseControl::reset_func)); 
        m_rest_timer->cancel();     
    }
    //生成随机点
    void random_point_output();
    //速度控制函数
    void ctl_func();
    //小海龟停止函数
    void reset_func();

};
void TurtlePoseControl::random_point_output(){
    this->goal_x=m_distrib(m_gen);
    this->goal_y=m_distrib(m_gen);    
}
void TurtlePoseControl::ctl_func(){
        double distance=sqrt((goal_x-current_x)*(goal_x-current_x)+(goal_y-current_y)*(goal_y-current_y));
        double distheta=std::atan2(goal_y - current_y, goal_x - current_x)-current_theta;
        //保持角差属于[-PI,PI]
        while (distheta > M_PI)  distheta -= 2.0 * M_PI;
        while (distheta < -M_PI) distheta += 2.0 * M_PI;

        geometry_msgs::msg::Twist ctr_msg;
        if(distance<0.05){
            //小海龟到达逻辑，等待并设置新目标点
            RCLCPP_INFO(get_logger(),"Now turtle1 is on destination!)");
            ctr_msg.linear.x = 0.0;
            ctr_msg.angular.z = 0.0;
            m_control_timer->cancel();
            m_rest_timer->reset();

        }else if(std::abs(distheta)>0.1){
            //小海龟根据角度差赋予角速度原地旋转阶段
            ctr_msg.linear.x=0.0;
            ctr_msg.angular.z=KP_ANGULAR*distheta;
        }else if(std::abs(distheta)<0.1){
            //小海龟补齐角度差后赋予线速度阶段
            if(KP_LINEAR * distance<=4.0){
                ctr_msg.linear.x=KP_LINEAR * distance;
            }else{
                ctr_msg.linear.x=4.0;
            }
            ctr_msg.angular.z = KP_ANGULAR * distheta;
        }
        m_turtle_control->publish(ctr_msg);
}
void TurtlePoseControl::reset_func(){
    m_rest_timer->cancel();
    random_point_output();
    RCLCPP_INFO(this->get_logger(), "休息结束！新目标点为：(%f,%f))！",goal_x,goal_y);
    m_control_timer->reset();
}

int main(int argc,char** argv){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<TurtlePoseControl>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}