import launch
import launch_ros

def generate_launch_description():

    ation_node_start_static_tf=launch_ros.actions.Node(
        package='cpp_tf',
        executable='static_tf',
        output='screen'
    )
    ation_node_start_dynamic_tf=launch_ros.actions.Node(
        package='cpp_tf',
        executable='dynamic_tf',
        output='screen'
    )
    ation_node_start_tf_listener=launch_ros.actions.Node(
        package='cpp_tf',
        executable='tf_listener',
        output='screen'
    )

    return launch.LaunchDescription([
        ation_node_start_static_tf,
        ation_node_start_dynamic_tf,
        ation_node_start_tf_listener
    ])