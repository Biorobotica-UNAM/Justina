#include "JustinaHRI.h"
#include <iostream>
#include <chrono>
#include <thread>

JustinaHRI::JustinaHRI(const rclcpp::Node::SharedPtr& node) : node_(node) {}

void JustinaHRI::say(const std::string& text, int priority)
{
    (void)priority;
    std::cout << "[HRI] " << text << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2)); // simula tiempo hablando
}

std::string JustinaHRI::lastRecognizedSpeech(const std::string& file)
{
    (void)file;
    std::this_thread::sleep_for(std::chrono::seconds(2)); // simula espera por comando
    return "continue";
}

