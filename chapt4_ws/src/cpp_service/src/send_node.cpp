//该节点作为Service端用于接收客户的范围(request)以生成随机数返回给客户(response)
#include "rclcpp/rclcpp.hpp"
#include "service_interfaces/srv/guess_service.hpp"
#include <random>
#include <cstdint>

class SendNode : public rclcpp::Node{
private:
    rclcpp::Service<service_interfaces::srv::GuessService>::SharedPtr mp_service;
    std::mt19937_64 m_gen;
public:
    SendNode():Node("service_node"),m_gen(std::random_device{}()){
        //初始化随机数并赋值逻辑
        mp_service=this->create_service<service_interfaces::srv::GuessService>("number_guess",
            [this](const service_interfaces::srv::GuessService::Request::SharedPtr request,service_interfaces::srv::GuessService::Response::SharedPtr response)->void{
                std::uniform_int_distribution<int64_t> dis(request->min_num,request->max_num);
                response->goal_num = dis(m_gen);
            });
    }
};

int main(int argc,char**argv){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<SendNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}
