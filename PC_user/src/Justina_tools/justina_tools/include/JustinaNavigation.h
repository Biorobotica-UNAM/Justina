#pragma once

#include <iostream>
#include <cmath>
#include <string>
#include <memory>
#include <vector>

#include "tf2_ros/buffer.h"
#include "tf2_ros/transform_listener.h"

#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "geometry_msgs/msg/pose_stamped.hpp"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "sensor_msgs/msg/laser_scan.hpp"
#include "std_msgs/msg/float32.hpp"
#include "std_msgs/msg/empty.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/float32_multi_array.hpp"

#include "simple_move/srv/move_base.hpp"
#include "std_srvs/srv/set_bool.hpp"

class JustinaNavigation : public rclcpp::Node
{
public:
    JustinaNavigation();

    // Get Robot Pose
    void getRobotPoseWrtMap(float& x, float& y, float& theta);
    void getRobotPoseWrtOdom(float& x, float& y, float& theta);

    // Command Movement To Base
    void startMoveDist(float distance);
    void startMoveDistAngle(float distance, float angle);

    // Unplanned Movement
    void moveBase(double x, double y, double theta, double timeout);

	// Align Base
    void alignWithLine(bool enable);

    // Move close using mvn_pln
    void startGetClose(float x, float y, float angle);
    void startGetClose(const std::string& location);
    bool getClose(float x, float y, float angle, int timeout_ms);
    bool getClose(const std::string& location, int timeout_ms);

    void stopNavigation();
    bool waitForDoor();
    void modifyMap(const std::string& zones);

private:
    // Subscribers
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr sub_stop_;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr sub_navigation_stop_;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr sub_simple_move_status_;
    rclcpp::Subscription<std_msgs::msg::Empty>::SharedPtr sub_navigation_status_;
    rclcpp::Subscription<sensor_msgs::msg::LaserScan>::SharedPtr sub_laser_scan_;

    // Publishers
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr pub_simple_move_dist_;
    rclcpp::Publisher<std_msgs::msg::Float32MultiArray>::SharedPtr pub_simple_move_dist_angle_;
    rclcpp::Publisher<std_msgs::msg::Float32>::SharedPtr pub_simple_move_lateral_;
    rclcpp::Publisher<geometry_msgs::msg::PoseStamped>::SharedPtr pub_mvn_pln_get_close_xya_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_mvn_pln_get_close_loc_;
    rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr pub_navigation_stop_;
    rclcpp::Publisher<geometry_msgs::msg::Twist>::SharedPtr pub_cmd_vel_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_mod_map_;

    // Service clients
    rclcpp::Client<simple_move::srv::MoveBase>::SharedPtr clt_move_base_;
    rclcpp::Client<std_srvs::srv::SetBool>::SharedPtr clt_align_with_line_;

    // TF listener
    std::unique_ptr<tf2_ros::Buffer> tf_buffer_;
    std::shared_ptr<tf2_ros::TransformListener> tf_listener_;

    // Internal state
    sensor_msgs::msg::LaserScan laser_scan_;
    bool stop_flag_;
};
