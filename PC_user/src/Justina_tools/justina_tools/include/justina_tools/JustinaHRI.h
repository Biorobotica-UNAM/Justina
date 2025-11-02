#pragma once

#include <string>
#include <mutex>
#include <memory>
#include <chrono>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/string.hpp"
#include "std_msgs/msg/bool.hpp"

class JustinaHRI
{
private:
    rclcpp::Node::SharedPtr node_;
    rclcpp::Publisher<std_msgs::msg::String>::SharedPtr pub_speech_;
    rclcpp::Subscription<std_msgs::msg::Bool>::SharedPtr sub_speech_status_;
    
    std::mutex mtx_;
    bool speech_done_;

    void speechStatusCallback(const std_msgs::msg::Bool::SharedPtr msg);
    
public:
    JustinaHRI(rclcpp::Node::SharedPtr node);
    
    void say(const std::string& text);
    std::string lastRecognizedSpeech(const std::string& file);
};
