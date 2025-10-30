#include "JustinaNavigation.h"

using namespace std::chrono_literals;

JustinaNavigation::JustinaNavigation() : Node("justina_navigation"), stop_flag_(false)
{
    // TF2
    tf_buffer_ = std::make_unique<tf2_ros::Buffer>(this->get_clock());
    tf_listener_ = std::make_shared<tf2_ros::TransformListener>(*tf_buffer_);

    // Subscribers
    sub_stop_ = this->create_subscription<std_msgs::msg::Empty>("/stop", 10,
        [this](std_msgs::msg::Empty::SharedPtr){ stop_flag_ = true; });

    sub_navigation_stop_ = this->create_subscription<std_msgs::msg::Empty>("/navigation/stop", 10,
        [this](std_msgs::msg::Empty::SharedPtr){ stop_flag_ = true; });

    sub_laser_scan_ = this->create_subscription<sensor_msgs::msg::LaserScan>("/scan", 10,
        [this](sensor_msgs::msg::LaserScan::SharedPtr msg){ laser_scan_ = *msg; });

    // Publishers
    pub_simple_move_dist_ = this->create_publisher<std_msgs::msg::Float32>("/simple_move/goal_dist", 10);
    pub_simple_move_dist_angle_ = this->create_publisher<std_msgs::msg::Float32MultiArray>("/simple_move/goal_dist_angle", 10);
    pub_simple_move_lateral_ = this->create_publisher<std_msgs::msg::Float32>("/simple_move/goal_dist_lateral", 10);
    pub_mvn_pln_get_close_xya_ = this->create_publisher<geometry_msgs::msg::PoseStamped>("/move_base_simple/goal", 10);
    pub_mvn_pln_get_close_loc_ = this->create_publisher<std_msgs::msg::String>("/navigation/mvn_pln/get_close_loc", 1);
    pub_navigation_stop_ = this->create_publisher<std_msgs::msg::Empty>("/navigation/stop", 10);
    pub_cmd_vel_ = this->create_publisher<geometry_msgs::msg::Twist>("/cmd_vel", 10);
    pub_mod_map_ = this->create_publisher<std_msgs::msg::String>("/zone_modifications", 1);

    // Service clients
    clt_move_base_ = this->create_client<simple_move::srv::MoveBase>("/navigation/move_base");
    clt_align_with_line_ = this->create_client<std_srvs::srv::SetBool>("/navigation/align_with_line");
}

// ===== Robot Pose =====
void JustinaNavigation::getRobotPoseWrtMap(float& x, float& y, float& theta)
{
    try {
        auto t = tf_buffer_->lookupTransform("map", "base_link", tf2::TimePointZero);
        x = t.transform.translation.x;
        y = t.transform.translation.y;
        theta = 2 * atan2(t.transform.rotation.z, t.transform.rotation.w);
    } catch (tf2::TransformException &ex) {
        RCLCPP_WARN(this->get_logger(), "TF lookup failed: %s", ex.what());
        x = y = theta = 0;
    }
}

void JustinaNavigation::getRobotPoseWrtOdom(float& x, float& y, float& theta)
{
    try {
        auto t = tf_buffer_->lookupTransform("odom", "base_link", tf2::TimePointZero);
        x = t.transform.translation.x;
        y = t.transform.translation.y;
        theta = 2 * atan2(t.transform.rotation.z, t.transform.rotation.w);
    } catch (tf2::TransformException &ex) {
        RCLCPP_WARN(this->get_logger(), "TF lookup failed: %s", ex.what());
        x = y = theta = 0;
    }
}

// ===== Simple Move =====
void JustinaNavigation::startMoveDist(float distance)
{
    std_msgs::msg::Float32 msg;
    msg.data = distance;
    pub_simple_move_dist_->publish(msg);
}

void JustinaNavigation::startMoveDistAngle(float distance, float angle)
{
    std_msgs::msg::Float32MultiArray msg;
    msg.data.push_back(distance);
    msg.data.push_back(angle);
    pub_simple_move_dist_angle_->publish(msg);
}

// ===== MoveBase =====
void JustinaNavigation::moveBase(double x, double y, double theta, double timeout)
{
    if (!clt_move_base_->wait_for_service(1s)) {
        RCLCPP_ERROR(this->get_logger(), "MoveBase service not available");
        return;
    }

    auto request = std::make_shared<simple_move::srv::MoveBase::Request>();
    request->x = x;
    request->y = y;
    request->theta = theta;
    request->time_out = timeout;

    auto result = clt_move_base_->async_send_request(request);
}

// ===== Align =====
void JustinaNavigation::alignWithLine(bool enable)
{
    if (!clt_align_with_line_->wait_for_service(1s)) {
        RCLCPP_ERROR(this->get_logger(), "Align service not available");
        return;
    }

    auto request = std::make_shared<std_srvs::srv::SetBool::Request>();
    request->data = enable;
    clt_align_with_line_->async_send_request(request);
}

// ===== GetClose =====
void JustinaNavigation::startGetClose(float x, float y, float angle)
{
    geometry_msgs::msg::PoseStamped msg;
    msg.header.frame_id = "map";
    msg.pose.position.x = x;
    msg.pose.position.y = y;
    msg.pose.orientation.z = sin(angle/2);
    msg.pose.orientation.w = cos(angle/2);
    pub_mvn_pln_get_close_xya_->publish(msg);
}

void JustinaNavigation::startGetClose(const std::string& location)
{
    std_msgs::msg::String msg;
    msg.data = location;
    pub_mvn_pln_get_close_loc_->publish(msg);
}

// ===== Stop Navigation =====
void JustinaNavigation::stopNavigation()
{
    std_msgs::msg::Empty msg;
    pub_navigation_stop_->publish(msg);
}

// ===== Modify Map =====
void JustinaNavigation::modifyMap(const std::string& zones)
{
    std_msgs::msg::String msg;
    msg.data = zones;
    pub_mod_map_->publish(msg);
}

// ===== Wait for Door =====
bool JustinaNavigation::waitForDoor()
{
    if (laser_scan_.ranges.empty()) return false;

    size_t range_c = laser_scan_.ranges.size() / 2;
    size_t range_i = range_c - laser_scan_.ranges.size()/10;
    size_t range_f = range_c + laser_scan_.ranges.size()/10;

    float sum = 0.0;
    int count = 0;

    for (size_t i = range_i; i < range_f; ++i)
    {
        if (laser_scan_.ranges[i] > 0 && laser_scan_.ranges[i] < 4)
        {
            sum += laser_scan_.ranges[i];
            count++;
        }
    }

    if (count == 0) return false;

    return (sum / count) > 0.5;
}
