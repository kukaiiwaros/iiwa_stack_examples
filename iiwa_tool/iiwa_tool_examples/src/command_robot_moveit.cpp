#include "ros/ros.h"
#include <iiwa_ros.h>
#include <moveit/move_group_interface/move_group_interface.h>

int main (int argc, char **argv) {
  
  // Initialize ROS
  ros::init(argc, argv, "CommandRobotMoveit");
  ros::NodeHandle nh("~");
  
  // ROS spinner.
  ros::AsyncSpinner spinner(1);
  spinner.start();
  
  iiwa_ros::iiwaRos my_iiwa;
  my_iiwa.init();
  
  std::string movegroup_name, ee_link;
  geometry_msgs::PoseStamped command_cartesian_position;
  double ros_rate = 0.1;
  bool isRobotConnected = false;
  
  // Dynamic parameters. Last arg is the default value. You can assign these from a launch file.
  nh.param<std::string>("move_group", movegroup_name, "manipulator");
  nh.param<std::string>("ee_link", ee_link, "tool_link_ee");
  
  // Dynamic parameter to choose the rate at wich this node should run
  nh.param("ros_rate", ros_rate, 0.1); // 0.1 Hz = 10 seconds
  ros::Rate* loop_rate_ = new ros::Rate(ros_rate);
    
  int direction = 1;
  
  // Create MoveGroup
  moveit::planning_interface::MoveGroupInterface group(movegroup_name);
  moveit::planning_interface::MoveGroupInterface::Plan myplan;
      
  // Configure planner 
  group.setPlanningTime(0.5);
  group.setPlannerId("RRTConnectkConfigDefault");
  group.setEndEffectorLink(ee_link);
  bool success_plan = false, motion_done = false, new_pose = false;
  while (ros::ok()) {
    if (my_iiwa.getRobotIsConnected()) {
      
      command_cartesian_position = group.getCurrentPose(ee_link);  
      command_cartesian_position.pose.position.z -= direction * 0.10;
  
      group.setStartStateToCurrentState();
      group.setPoseTarget(command_cartesian_position, ee_link);
      success_plan = group.plan(myplan);
      if (success_plan) {
        motion_done = group.execute(myplan);
      }
      if (motion_done) {
        direction *= -1; // In the next iteration the motion will be on the opposite direction
        loop_rate_->sleep(); // Sleep for some millisecond. The while loop will run every 10 seconds in this example.
      }
    }
    else {
      ROS_WARN_STREAM("Robot is not connected...");
      ros::Duration(5.0).sleep(); // 5 seconds
    }
  }  
}; 