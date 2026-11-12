import launch
import launch_ros
from ament_index_python.packages import get_package_share_directory # 找目录包
import os

def generate_launch_description():
    # 获取包路径
    urdf_package_path = get_package_share_directory('fishbot_description') 

    # 获取xacro文件路径
    default_urdf_path=os.path.join(urdf_package_path,'urdf','fishbot','fishbot.urdf.xacro') 

    # 获取默认rviz配置文件路径
    default_rviz_config_path=os.path.join(urdf_package_path,'config','display_robot_model.rviz')

    # 声明参数
    action_declare_arg_mode_path = launch.actions.DeclareLaunchArgument(
        name='model', default_value=str(default_urdf_path), description='加载的模型文件路径'
    )
    # 使用Command substitution将xacro文件转换为urdf文件
    substitutinos_command_result=launch.substitutions.Command(['xacro ',launch.substitutions.LaunchConfiguration('model')])
    # 将urdf文件内容作为参数传入robot_state_publisher节点
    robot_description_value=launch_ros.parameter_descriptions.ParameterValue(substitutinos_command_result, value_type=str)

    action_robot_state_publisher = launch_ros.actions.Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        parameters=[{'robot_description':robot_description_value}]
    )
    action_joint_state_publisher = launch_ros.actions.Node(
        package='joint_state_publisher',
        executable='joint_state_publisher'
    )

    action_rviz_node = launch_ros.actions.Node(
        package='rviz2',
        executable='rviz2',
        arguments=['-d', default_rviz_config_path]
    )
    
    return launch.LaunchDescription([
        action_declare_arg_mode_path,
        action_robot_state_publisher, 
        action_joint_state_publisher,
        action_rviz_node
        ])