#include "justina_tools/JustinaHRI.h"

JustinaHRI::JustinaHRI(rclcpp::Node::SharedPtr node)
    : node_(node), speech_done_(false)
{
    pub_speech_ = node_->create_publisher<std_msgs::msg::String>("voice/speech", 10);
    sub_speech_status_ = node_->create_subscription<std_msgs::msg::Bool>("voice/speech_status", 10, std::bind(&JustinaHRI::speechStatusCallback, this, std::placeholders::_1));
}

void JustinaHRI::speechStatusCallback(const std_msgs::msg::Bool::SharedPtr msg)
{
    if(msg->data)
    {
        std::lock_guard<std::mutex> lock(mtx_);
        speech_done_ = true;
    }
}

void JustinaHRI::say(const std::string& text)
{
    while(pub_speech_->get_subscription_count() == 0 && rclcpp::ok())
    {
        rclcpp::spin_some(node_);
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }

    std_msgs::msg::String msg;
    msg.data = text;

    {
        std::lock_guard<std::mutex> lock(mtx_);
        speech_done_ = false;
    }

    pub_speech_->publish(msg);

    while(rclcpp::ok())
    {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            if(speech_done_)
                break;
        }
        rclcpp::spin_some(node_);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

std::string JustinaHRI::lastRecognizedSpeech(const std::string&)
{
    return "continue";
}

