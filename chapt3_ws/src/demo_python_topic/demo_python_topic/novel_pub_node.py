import rclpy
from rclpy.node import Node
import requests
from example_interfaces.msg import String
from queue import Queue

class NovelPubNode(Node):
    def __init__(self):
        super().__init__('novel_pub')
        self.get_logger().info(f'novel_pub_node initialized')
        self.novel_publisher_=self.publisher_ = self.create_publisher(String, 'novel_topic', 10)
        self.novel_queue_=Queue()
        self.create_timer(5.0,self.timer_callback)

    def download(self,url):
        response = requests.get(url)
        response.encoding='utf-8'
        if response.status_code == 200:
            novel_text = response.text
            self.get_logger().info(f'Novel downloaded successfully')
            for line in novel_text.splitlines():
                self.novel_queue_.put(line)
        else:
            self.get_logger().error(f'Failed to download novel. Status code: {response.status_code}')

    def timer_callback(self):
        if self.novel_queue_.qsize()>0:
            line=self.novel_queue_.get()
            msg=String()
            msg.data=line
            self.novel_publisher_.publish(msg)
        

def main():
    rclpy.init()
    node=NovelPubNode()
    node.download('http://0.0.0.0:8000/novel1.txt')
    rclpy.spin(node)
    node.destroy_node()
    rclpy.shutdown()