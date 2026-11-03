//用于练习服务回调函数；参数通信；多节点启动
//该节点为客户端节点
#include "service_interfaces/srv/practise_service.hpp"
#include "rclcpp/rclcpp.hpp"
#include <chrono>
class ClientNode : public rclcpp::Node{

public:
    ClientNode():Node("std_client_node"){
        //参数声明逻辑未完成!!

        mp_client=this->create_client<service_interfaces::srv::PractiseService>("practise_service");
        while(!mp_client->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(), "等待服务端上线中...");
        }
        auto request=std::make_shared<service_interfaces::srv::PractiseService::Request>();
        request->req_num=m_request;
        mp_client->async_send_request(request,[this](rclcpp::Client<service_interfaces::srv::PractiseService>::SharedFuture future) -> void {
            auto response = future.get(); // 从 future 凭证中取出响应结果
            RCLCPP_INFO(this->get_logger(),"服务端响应为:%d",response->res_num);
        });
    }

private:
    rclcpp::Client<service_interfaces::srv::PractiseService>::SharedPtr mp_client;
    int m_request=1;
};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<ClientNode>();   
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}