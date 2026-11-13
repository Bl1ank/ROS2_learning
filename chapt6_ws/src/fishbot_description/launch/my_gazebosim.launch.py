import os
import launch
import launch_ros
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.parameter_descriptions import ParameterValue

# !!! todo: 1.添加世界模型路径 2.添加spawn时序 3.判断是否需要bridge桥接 4. 添加其他需要的参数
def generate_launch_description():
    pkg_share = get_package_share_directory('fishbot_description')
    default_model_path = os.path.join(pkg_share, 'urdf', 'fishbot', 'fishbot.urdf.xacro')
    defualt_world_path = os.path.join(pkg_share, 'worlds', 'world.sdf')
    declare_model = DeclareLaunchArgument(
        name='model', default_value=default_model_path,description='加载的模型文件路径')

    declare_world = DeclareLaunchArgument(
        name='world', default_value=defualt_world_path, description='加载的世界文件路径')
    
    robot_description = ParameterValue(Command(['xacro ', LaunchConfiguration('model')]), value_type=str)

    robot_state_publisher = launch_ros.actions.Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}],
        output='screen')

    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')),
        launch_arguments={
            'gz_args': ['-r -v 4 ', LaunchConfiguration('world')],   # -r 启动即运行
        }.items())

    spawn_entity = launch_ros.actions.Node(
        package='ros_gz_sim',
        executable='create',
        arguments=['-topic', 'robot_description', '-name', 'fishbot', '-z', '0.3'],
        output='screen')

    bridge = launch_ros.actions.Node(
    package='ros_gz_bridge',
    executable='parameter_bridge',
    arguments=[
        '/cmd_vel@geometry_msgs/msg/Twist]gz.msgs.Twist',   # ROS -> Gazebo
        '/odom@nav_msgs/msg/Odometry[gz.msgs.Odometry',     # Gazebo -> ROS
        '/tf@tf2_msgs/msg/TFMessage[gz.msgs.Pose_V',        # 可选，给 RViz 用
        '/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock',
    ],
    output='screen')
    return launch.LaunchDescription([
        declare_model,
        declare_world,
        robot_state_publisher,
        gz_sim,
        spawn_entity,
        bridge
    ])