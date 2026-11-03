//用于练习服务回调函数；参数通信；多节点启动
//该节点为服务端节点
#include "service_interfaces/srv/practise_service.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"


class ServiceNode : public rclcpp::Node{
public:
    ServiceNode():Node("std_service_node"){
        //参数声明逻辑未完成!!
        
        // this->declare_parameter<std::string>("m_response","res_num");
        // mp_parameter_callback_handle=this->add_on_set_parameters_callback([this](const std::vector<rclcpp::Parameter> & parameters)->rcl_interfaces::msg::SetParametersResult{
        //     rcl_interfaces::msg::SetParametersResult result;
        //     result.successful=true;
        //     for(const auto & parameter:parameters){
        //         //判断参数合法并赋值逻辑
        //     }
        //     return result;
        // });
        //初始化服务节点
        mp_service=this->create_service<service_interfaces::srv::PractiseService>("practise_service",[this](const service_interfaces::srv::PractiseService::Request::SharedPtr req, service_interfaces::srv::PractiseService::Response::SharedPtr res)->void{
            RCLCPP_INFO(this->get_logger(),"接收到客户端请求:");
            RCLCPP_INFO(this->get_logger(),"服务端正在处理请求...");
            m_response=req->req_num;
            res->res_num=m_response;                    
        });

    }

private:
    rclcpp::Service<service_interfaces::srv::PractiseService>::SharedPtr mp_service;
    OnSetParametersCallbackHandle::SharedPtr mp_parameter_callback_handle;
    int m_response;

};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<ServiceNode>();  
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}