//用于练习服务回调函数；参数通信；多节点启动
//该节点为客户端节点
#include "service_interfaces/srv/practise_service.hpp"
#include "rclcpp/rclcpp.hpp"
#include <chrono>
class ClientNode : public rclcpp::Node{

public:
    ClientNode():Node("std_client_node"){
        //参数描述
        rcl_interfaces::msg::ParameterDescriptor req_descriptor;
        req_descriptor.description="客户端请求参数";
        rcl_interfaces::msg::IntegerRange req_range;
        req_range.from_value=0;
        req_range.to_value=64;
        req_range.step=1;
        req_descriptor.integer_range.push_back(req_range);
        //参数声明
        this->declare_parameter<int>("m_request",0,req_descriptor);
        //参数更新回调句柄
        mp_parameter_callback_handle=this->add_on_set_parameters_callback([this](const std::vector<rclcpp::Parameter> & parameters)->rcl_interfaces::msg::SetParametersResult{
            rcl_interfaces::msg::SetParametersResult result;
            result.successful=true;
            for(const auto & parameter:parameters){
                //判断参数合法并赋值逻辑
                if(parameter.get_name()=="m_request"){
                    int value=parameter.as_int();
                    if(value<0||value>64){
                        result.successful=false;
                        result.reason="value must be in [0,64]";
                    }else{
                        result.successful=true;
                        result.reason="value is valid";
                        RCLCPP_INFO(this->get_logger(),"客户端请求参数m_request被修改为:%d",value);
                        m_request=value;
                        //更新参数后自动请求响应逻辑
                        auto request=std::make_shared<service_interfaces::srv::PractiseService::Request>();
                        request->req_num=m_request;
                        mp_client->async_send_request(request,[this](rclcpp::Client<service_interfaces::srv::PractiseService>::SharedFuture future) -> void {
                            auto response = future.get(); // 从 future 凭证中取出响应结果
                            RCLCPP_INFO(this->get_logger(),"服务端响应为:%d",response->res_num);
                        });
                    }
                }
            }
            return result;
        });
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
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr mp_parameter_callback_handle;
    int m_request=1;
};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<ClientNode>();   
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}