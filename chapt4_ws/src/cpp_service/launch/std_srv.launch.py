import launch
import launch_ros

def generate_launch_description():

    ation_node_start_service=launch_ros.actions.Node(
        package='cpp_service',
        executable='service_node',
        output='screen'
    )
    ation_node_start_client=launch_ros.actions.Node(
        package='cpp_service',
        executable='client_node',
        output='screen'
    )

    return launch.LaunchDescription([
        ation_node_start_service,
        ation_node_start_client
    ])