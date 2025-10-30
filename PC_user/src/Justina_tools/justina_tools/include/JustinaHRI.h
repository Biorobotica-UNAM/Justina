#pragma once
#include <iostream>
#include <string>
#include <vector>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"
#include "std_msgs/msg/empty.hpp"
#include "vosk_speech_recognition/srv/speech_recog.hpp"

class JustinaHRI
{
private:
    static bool is_node_set;

    // Node pointer
    static rclcpp::Node::SharedPtr node;

    // Speaker
    static rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pubSpeaker;

    // Leg Finder
    static rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr pubLegsFinderEnable;
    static rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr subLegsFinderFound;
    static bool _legsFound;

    // Human Follower
    static rclcpp::Publisher<std_msgs::msg::Bool>::SharedPtr pubHumanFollowerEnable;
    static rclcpp::Publisher<std_msgs::msg::Empty>::SharedPtr pubHumanFollowerStop;

    // Vosk Speech Recognition
    static rclcpp::Client<vosk_speech_recognition::srv::SpeechRecog>::SharedPtr cltVoskRecog;

    // Callback
    static void callbackLegsFound(const std_msgs::msg::Bool::SharedPtr msg);

public:
    static bool setNode(rclcpp::Node::SharedPtr nh);

    // Speaker
    static void say(const std::string& strToSay, int timeout);

    // Leg Finder
    static void enableLegFinder(bool enable);
    static bool frontalLegsFound();

    // Human Follower
    static void enableHumanFollower(bool enable);
    static void stopHumanFollower();

    // Speech recognition
    static std::string lastRecogSpeech(const std::string& grammar);
};
