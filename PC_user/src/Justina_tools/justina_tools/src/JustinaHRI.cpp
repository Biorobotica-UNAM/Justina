#include "JustinaHRI.h"
#include <chrono>

using namespace std::chrono_literals;

bool JustinaHRI::is_node_set = false;
rclcpp::Node::SharedPtr JustinaHRI::node = nullptr;

// Speaker
rclcpp::Publisher<std_msgs::msg::String>::SharedPtr JustinaHRI::pubSpeaker = nullptr;

// Leg Finder
rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr JustinaHRI::pubLegsFinderEnable = nullptr;
rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr JustinaHRI::subLegsFinderFound = nullptr;
bool JustinaHRI::_legsFound = false;

// Human Follower
rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr JustinaHRI::pubHumanFollowerEnable = nullptr;
rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr JustinaHRI::pubHumanFollowerStop = nullptr;

// Vosk Speech Recognition
rclcpp::Client<vosk_speech_recognition::srv::SpeechRecog>::SharedPtr JustinaHRI::cltVoskRecog = nullptr;

// Callback
void JustinaHRI::callbackLegsFound(const std_msgs::msg::Bool::SharedPtr msg)
{
    _legsFound = msg->data;
}

// Node setup
bool JustinaHRI::setNode(rclcpp::Node::SharedPtr nh)
{
    if (is_node_set) return true;
    if (!nh) return false;

    node = nh;

    RCLCPP_INFO(node->get_logger(), "JustinaHRI -> Setting ROS2 node...");

    // Speaker
    pubSpeaker = node->create_publisher<std_msgs::msg::String>("/speak", 10);

    // Leg Finder
    pubLegsFinderEnable = node->create_publisher<std_msgs::msg::Bool>("/hri/leg_finder/enable", 10);
    subLegsFinderFound  = node->create_subscription<std_msgs::msg::Bool>(
        "/hri/leg_finder/legs_found", 10, callbackLegsFound
    );

    // Human Follower
    pubHumanFollowerEnable = node->create_publisher<std_msgs::msg::Bool>("/hri/human_following/start_follow", 10);
    pubHumanFollowerStop   = node->create_publisher<std_msgs::msg::Empty>("/hri/human_following/stop", 10);

    // Vosk
    cltVoskRecog = node->create_client<vosk_speech_recognition::srv::SpeechRecog>("/hri/speech_recognition");

    is_node_set = true;
    return true;
}

// Speaker
void JustinaHRI::say(const std::string& strToSay, int timeout)
{
    if (!pubSpeaker) return;

    RCLCPP_INFO(node->get_logger(), "JustinaHRI -> Saying: %s", strToSay.c_str());

    auto msg = std_msgs::msg::String();
    msg.data = strToSay;
    pubSpeaker->publish(msg);

    rclcpp::sleep_for(std::chrono::seconds(timeout));
}

// Leg Finder
void JustinaHRI::enableLegFinder(bool enable)
{
    if (!pubLegsFinderEnable) return;

    RCLCPP_INFO(node->get_logger(), "JustinaHRI -> Leg finder %s", enable ? "enabled" : "disabled");
    auto msg = std_msgs::msg::Bool();
    msg.data = enable;
    pubLegsFinderEnable->publish(msg);
}

bool JustinaHRI::frontalLegsFound()
{
    return _legsFound;
}

// Human Follower
void JustinaHRI::enableHumanFollower(bool enable)
{
    if (!pubHumanFollowerEnable) return;

    RCLCPP_INFO(node->get_logger(), "JustinaHRI -> Human follower %s", enable ? "enabled" : "disabled");
    auto msg = std_msgs::msg::Bool();
    msg.data = enable;
    pubHumanFollowerEnable->publish(msg);
}

void JustinaHRI::stopHumanFollower()
{
    if (!pubHumanFollowerStop) return;

    auto msg = std_msgs::msg::Empty();
    pubHumanFollowerStop->publish(msg);
}

// Speech recognition
std::string JustinaHRI::lastRecogSpeech(const std::string& grammar)
{
    if (!cltVoskRecog) return "";

    RCLCPP_INFO(node->get_logger(), "JustinaHRI -> Last Recognized Speech");

    auto request = std::make_shared<vosk_speech_recognition::srv::SpeechRecog::Request>();
    request->is_speech_recog_enabled = true;
    request->grammar = grammar;

    // Wait for service
    if (!cltVoskRecog->wait_for_service(1s)) {
        RCLCPP_WARN(node->get_logger(), "JustinaHRI -> Vosk service not available");
        return "";
    }

    auto result_future = cltVoskRecog->async_send_request(request);
    if (rclcpp::spin_until_future_complete(node, result_future) ==
        rclcpp::FutureReturnCode::SUCCESS) {
        return result_future.get()->text_recog;
    } else {
        RCLCPP_WARN(node->get_logger(), "JustinaHRI -> Speech recognition failed");
        return "";
    }
}
