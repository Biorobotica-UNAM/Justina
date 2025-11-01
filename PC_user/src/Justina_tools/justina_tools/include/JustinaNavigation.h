#pragma once
#include "rclcpp/rclcpp.hpp"
#include <vector>
#include <string>

class JustinaNavigation
{
public:
    explicit JustinaNavigation(const rclcpp::Node::SharedPtr& node);

    bool getClose(float x, float y, float theta, int timeout_ms);

    std::vector<float> getCoordinates(const std::string& location);

private:
    rclcpp::Node::SharedPtr node_;
};
