"""
在 Gazebo Harmonic 中加载 fishbot 模型。

流程:
  1. xacro  -> URDF  (调用 xacro 命令展开宏)
  2. URDF   -> SDF   (调用 gz sdf -p, 即 sdformat 的 URDF 解析器)
  3. 启动 Gazebo Harmonic, 用 ros_gz_sim create 把生成的 SDF 加载进世界

用法:
  ros2 launch fishbot_description gazebo.launch.py
  ros2 launch fishbot_description gazebo.launch.py headless:=true
  ros2 launch fishbot_description gazebo.launch.py world:=/path/to/my_world.sdf
"""
import os
import subprocess

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import (
    AppendEnvironmentVariable,
    DeclareLaunchArgument,
    IncludeLaunchDescription,
    LogInfo,
    OpaqueFunction,
)
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def _run_command(cmd, description):
    """执行外部命令, 失败时把 stderr 一起抛出, 方便定位 xacro/SDF 转换问题。"""
    result = subprocess.run(cmd, capture_output=True, text=True)
    if result.returncode != 0:
        raise RuntimeError(f'{description} 失败:\n{result.stderr}')
    return result.stdout


def launch_setup(context, *args, **kwargs):
    model_path = os.path.expanduser(LaunchConfiguration('model').perform(context))
    world_path = os.path.expanduser(LaunchConfiguration('world').perform(context))
    output_dir = os.path.expanduser(LaunchConfiguration('output_dir').perform(context))
    headless = LaunchConfiguration('headless').perform(context).lower() in ('true', '1', 'yes')
    use_sim_time = LaunchConfiguration('use_sim_time').perform(context).lower() in ('true', '1', 'yes')

    # 中间文件输出目录, 默认放到 ~/.cache, 方便查看转换结果且不污染 install 空间
    os.makedirs(output_dir, exist_ok=True)
    urdf_path = os.path.join(output_dir, 'fishbot.urdf')
    sdf_path = os.path.join(output_dir, 'fishbot.sdf')

    # 1. xacro -> URDF
    urdf_xml = _run_command(['xacro', model_path], 'xacro 展开')
    with open(urdf_path, 'w', encoding='utf-8') as f:
        f.write(urdf_xml)

    # 2. URDF -> SDF (Gazebo Harmonic 使用的 SDFormat 1.11)
    sdf_xml = _run_command(['gz', 'sdf', '-p', urdf_path], 'URDF 转 SDF')
    with open(sdf_path, 'w', encoding='utf-8') as f:
        f.write(sdf_xml)

    # 3. 用 URDF 发布 TF 树, 让 Gazebo 中的模型和 ROS 坐标系对应起来
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{
            'robot_description': urdf_xml,
            'use_sim_time': use_sim_time,
        }],
        output='screen',
    )

    # 4. 启动 Gazebo Harmonic(ros_gz_sim 的 gz_sim.launch.py)
    gz_args = ['-s ', world_path] if headless else [world_path]
    gz_sim = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(get_package_share_directory('ros_gz_sim'), 'launch', 'gz_sim.launch.py')
        ),
        launch_arguments={'gz_args': ['-r -v4 ', *gz_args]}.items(),
    )

    # 5. 把 SDF 模型生成到 Gazebo 世界中
    spawn_model = Node(
        package='ros_gz_sim',
        executable='create',
        arguments=[
            '-file', sdf_path,
            '-name', 'fishbot',
            '-z', '0.05',  # 抬高一点, 避免轮子初始穿地
        ],
        output='screen',
    )

    # 6. 把 Gazebo 的仿真时钟桥接到 ROS 2, 否则 use_sim_time 的节点时间不走
    clock_bridge = Node(
        package='ros_gz_bridge',
        executable='parameter_bridge',
        arguments=['/clock@rosgraph_msgs/msg/Clock[gz.msgs.Clock'],
        output='screen',
    )

    # 7. 让 Gazebo 能找到 package:// 形式的资源(以后加 mesh 时有用)
    resource_path = AppendEnvironmentVariable(
        'GZ_SIM_RESOURCE_PATH',
        os.path.dirname(get_package_share_directory('fishbot_description')),
    )

    return [
        resource_path,
        LogInfo(msg=f'已生成 URDF: {urdf_path}'),
        LogInfo(msg=f'已生成 SDF : {sdf_path}'),
        robot_state_publisher,
        clock_bridge,
        gz_sim,
        spawn_model,
    ]


def generate_launch_description():
    urdf_package_path = get_package_share_directory('fishbot_description')
    default_model_path = os.path.join(
        urdf_package_path, 'urdf', 'fishbot', 'fishbot.urdf.xacro'
    )
    default_output_dir = os.path.join(
        os.path.expanduser('~'), '.cache', 'fishbot_description'
    )

    return LaunchDescription([
        DeclareLaunchArgument(
            name='model',
            default_value=default_model_path,
            description='要加载的 xacro/urdf 模型文件路径',
        ),
        DeclareLaunchArgument(
            name='world',
            default_value='empty.sdf',
            description='Gazebo 世界文件, 默认使用内置空世界',
        ),
        DeclareLaunchArgument(
            name='headless',
            default_value='false',
            description='是否只启动 Gazebo 服务端(不显示 GUI)',
        ),
        DeclareLaunchArgument(
            name='use_sim_time',
            default_value='true',
            description='ROS 节点是否使用 Gazebo 仿真时间',
        ),
        DeclareLaunchArgument(
            name='output_dir',
            default_value=default_output_dir,
            description='xacro 转出的 URDF/SDF 中间文件存放目录',
        ),
        OpaqueFunction(function=launch_setup),
    ])