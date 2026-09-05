#include<iostream>
#include"rclcpp/rclcpp.hpp"
#include<algorithm>
#include<vector>

int main(int argc,char **argv){
    std::cout<<"参数数量="<<argc<<std::endl;
    std::cout<<"参数列表:"<<std::endl;
    for(int i=0;i<argc;i++){
        std::cout<<"  "<<argv[i]<<std::endl;
    }
    std::vector<std::string> args(argv+1,argv+argc);
    if (std::find(args.begin(), args.end(), "--help") != args.end()){
        std::cout<<"this is help message"<<std::endl;
    };
    rclcpp::init(argc,argv);
    auto node = rclcpp::Node::make_shared("demo_cpp_node");
    RCLCPP_INFO(node->get_logger(),"Hello ROS2 C++");
    RCLCPP_WARN(node->get_logger(),"This is a warning message");
    RCLCPP_ERROR(node->get_logger(),"This is an error message");
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}