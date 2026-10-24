//该节点用于向Service端发送随机数范围请求(request)，接收到随机数(response)并用特定算法逼近得到结果
//流程：request->response->等待用户更改参数->request->response
#include "rclcpp/rclcpp.hpp"
#include "service_interfaces/srv/guess_service.hpp"
#include "rcl_interfaces/msg/set_parameters_result.hpp"
#include <chrono>
#include <limits>

class GuessNode : public rclcpp::Node{
private:
    int64_t m_min;
    int64_t m_max;
    rclcpp::Client<service_interfaces::srv::GuessService>::SharedPtr mp_client;
    //声明修改参数的结果
    rcl_interfaces::msg::SetParametersResult m_result;
    //声明参数更新事件句柄
    rclcpp::node_interfaces::OnSetParametersCallbackHandle::SharedPtr mp_parameter_callback;

public:
    GuessNode():Node("client_node"){
        mp_client=this->create_client<service_interfaces::srv::GuessService>("number_guess");
        while(!mp_client->wait_for_service(std::chrono::seconds(1))){
            RCLCPP_WARN(this->get_logger(), "等待服务端上线中...");
        }
        auto request=std::make_shared<service_interfaces::srv::GuessService::Request>();

        //[10-1000]
        request->min_num=10;
        request->max_num=1000;

        rcl_interfaces::msg::ParameterDescriptor downlimitation;
        downlimitation.description="随机数生成下限参数";
        rcl_interfaces::msg::IntegerRange down_range;
        down_range.from_value=0;
        down_range.to_value=INT64_MAX;
        down_range.step=1;
        downlimitation.integer_range.push_back(down_range);

        this->declare_parameter<int64_t>("litnum",10,downlimitation);

        rcl_interfaces::msg::ParameterDescriptor uplimitation;
        uplimitation.description="随机数生成上限参数";
        rcl_interfaces::msg::IntegerRange up_range;
        up_range.from_value=request->min_num;
        up_range.to_value=INT64_MAX;
        up_range.step=1;
        uplimitation.integer_range.push_back(up_range);
        
        this->declare_parameter<int64_t>("bignum",1000,uplimitation);
        //完成参数的声明;ros2参数和代码参数链接
        this->get_parameter("litnum",request->min_num);
        this->get_parameter("bignum",request->max_num);
        
        mp_client->async_send_request(request,[this,request](rclcpp::Client<service_interfaces::srv::GuessService>::SharedFuture future) -> void {
        auto response = future.get(); // 从 future 凭证中取出响应结果
        //猜测和计数逻辑
        int64_t guess=(int64_t)(request->min_num+request->max_num)/2;
        int64_t minindex=request->min_num;
        int64_t maxindex=request->max_num;
        int64_t times=1;
        while(guess!=response->goal_num){
            if(response->goal_num>guess){
                minindex=guess;
                guess=(int64_t)(minindex+maxindex)/2;
            }else if(response->goal_num<guess){
                maxindex=guess;
                guess=(int64_t)(minindex+maxindex)/2;
            }
            RCLCPP_INFO(get_logger(),"第%ld次猜测的数字为:%ld",times,guess);
            times+=1;
        }
        //播报逻辑
        RCLCPP_INFO(get_logger(),"response=%ld,guessnum=%ld,guesstimes=%ld",response->goal_num,guess,times-1);
        });
        //2.参数更新事件，包含发布新参数和重新生成并猜测随机数
        mp_parameter_callback=this->add_on_set_parameters_callback(
            [this](const std::vector<rclcpp::Parameter> & parameters)->rcl_interfaces::msg::SetParametersResult{
                
                m_result.successful=true;
                for(const auto & parameter:parameters){
                    if(parameter.get_name()=="litnum"){
                        m_min=parameter.as_int();
                    }else if(parameter.get_name()=="bignum"){
                        m_max=parameter.as_int();
                    }
                }
                RCLCPP_INFO(get_logger(),"参数更新成功,范围为[%ld,%ld]",m_min,m_max);
                return m_result;
            });
                

        
    }
};

int main(int argc,char**argv){
    rclcpp::init(argc,argv);
    auto node = std::make_shared<GuessNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}