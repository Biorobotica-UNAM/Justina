#pragma once
#include "rclcpp/rclcpp.hpp"
#include <string>

class JustinaHRI
{
public:
    explicit JustinaHRI(const rclcpp::Node::SharedPtr& node);
    void say(const std::string& text, int priority);
    std::string lastRecognizedSpeech(const std::string& file);

private:
    rclcpp::Node::SharedPtr node_;
};

