//用于练习服务回调函数；参数通信；多节点启动
//该节点为服务端节点
#include "service_interfaces/srv/practise_service.hpp"
#include "rclcpp/rclcpp.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include "rcl_interfaces/msg/parameter_descriptor.hpp"
#include "rcl_interfaces/msg/integer_range.hpp"

class ServiceNode : public rclcpp::Node{
public:
    ServiceNode():Node("std_service_node"){
        //参数描述
        rcl_interfaces::msg::ParameterDescriptor res_descriptor;
        res_descriptor.description="服务端响应参数";
        rcl_interfaces::msg::IntegerRange res_range;
        res_range.from_value=0;
        res_range.to_value=64;
        res_range.step=1;
        res_descriptor.integer_range.push_back(res_range);
        //参数声明
        this->declare_parameter<int>("m_response",0,res_descriptor);
        //参数更新回调句柄
        mp_parameter_callback_handle=this->add_on_set_parameters_callback([this](const std::vector<rclcpp::Parameter> & parameters)->rcl_interfaces::msg::SetParametersResult{
            rcl_interfaces::msg::SetParametersResult result;
            result.successful=true;
            for(const auto & parameter:parameters){
                //判断参数合法并赋值逻辑
                if(parameter.get_name()=="m_response"){
                    int value=parameter.as_int();
                    if(value<0||value>64){
                        result.successful=false;
                        result.reason="value must be in [0,64]";
                    }else{
                        result.successful=true;
                        result.reason="value is valid";
                        RCLCPP_INFO(this->get_logger(),"服务端响应参数m_response被修改为:%d",value);
                        m_response=value;
                    }
                }
            }
            return result;
        });
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
    //回调生命周期句柄
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr mp_parameter_callback_handle;
    //业务私有变量
    int m_response;

};

int main(int argc, char **argv){
    rclcpp::init(argc,argv);
    auto node=std::make_shared<ServiceNode>();  
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}