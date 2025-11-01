#include "JustinaNavigation.h"
#include <iostream>
#include <chrono>
#include <thread>

JustinaNavigation::JustinaNavigation(const rclcpp::Node::SharedPtr& node) : node_(node) {}

bool JustinaNavigation::getClose(float x, float y, float theta, int timeout_ms)
{
    (void)x;
    (void)y;
    (void)theta;
    std::cout << "[NAV] Moving to (" << x << ", " << y << ", " << theta << ")..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms / 10)); // simula movimiento
    std::cout << "[NAV] Arrived." << std::endl;
    return true;
}

std::vector<float> JustinaNavigation::getCoordinates(const std::string& location)
{
    std::cout << "[NAV] Requesting coordinates for: " << location << std::endl;
    if (location == "entrance_door") return {1.2f, 0.8f, 0.0f};
    if (location == "inspection_point") return {0.0f, 0.0f, 1.57f};
    return {0.0f, 0.0f, 0.0f};
}

