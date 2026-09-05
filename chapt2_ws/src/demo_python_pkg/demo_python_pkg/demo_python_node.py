import rclpy 
from rclpy.node import Node

def main():
    rclpy.init()
    node=Node("demo_python_node")
    node.get_logger().info("Hello ROS2")
    node.get_logger().warn("This is a warning message")
    node.get_logger().error("This is an error message")
    rclpy.spin(node)
    rclpy.shutdown()

# 放入功能包后直接在setup.py中设置入口点即可，不需要在这里写main函数
# if __name__=="__main__":
#     main()