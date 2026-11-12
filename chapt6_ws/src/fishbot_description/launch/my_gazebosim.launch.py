import os
import launch
import launch_ros
from ament_index_python.packages import get_package_share_directory
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command, LaunchConfiguration
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    pkg_share = get_package_share_directory('fishbot_description')
    default_model_path = os.path.join(pkg_share, 'urdf', 'fishbot', 'fishbot.urdf.xacro')

    declare_model = DeclareLaunchArgument(
        name='model', default_value=default_model_path,description='加载的模型文件路径')

    robot_description = ParameterValue(Command(['xacro ', LaunchConfiguration('model')]), value_type=str)

    robot_state_publisher = launch_ros.actions.Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description': robot_description}],
        output='screen')

    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(os.path.join(
            get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')),
        launch_arguments={'gz_args': '-r empty.sdf'}.items())

    spawn_entity = launch_ros.actions.Node(
        package='ros_gz_sim',
        executable='create',
        arguments=['-topic', 'robot_description', '-name', 'fishbot', '-z', '0.1'],
        output='screen')

    return launch.LaunchDescription([
        declare_model,
        robot_state_publisher,
        gz_sim,
        spawn_entity,
    ])