#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "JustinaHRI.h"
#include "JustinaNavigation.h"
#include "JustinaKnowledge.h"

enum SMState
{
    SM_INIT,
    SM_NAVIGATE_TO_INSPECTION,
    SM_WAIT_FOR_COMMAND,
    SM_NAVIGATE_TO_EXIT,
    SM_FINAL
};

int main(int argc, char **argv)
{
    std::cout << "=== INITIALIZING ROBOT INSPECTION ===" << std::endl;

    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("robot_inspection");

    auto hri = std::make_shared<JustinaHRI>(node);
    auto nav = std::make_shared<JustinaNavigation>(node);
    auto knowledge = std::make_shared<JustinaKnowledge>(node);

    rclcpp::Rate loop(10);
    SMState state = SM_INIT;

    std::vector<float> goal(3);
    std::string command;
    bool finished = false;

    while (rclcpp::ok() && !finished)
    {
        switch (state)
        {
            case SM_INIT:
                std::cout << "[SM_INIT]" << std::endl;
                hri->say("I am ready for robot inspection", 3);
                state = SM_NAVIGATE_TO_INSPECTION;
                break;

            case SM_NAVIGATE_TO_INSPECTION:
                std::cout << "[SM_NAVIGATE_TO_INSPECTION]" << std::endl;
                hri->say("I am going to the inspection point", 3);
                rclcpp::sleep_for(std::chrono::seconds(3));

                goal = knowledge->getCoordinates("entrance_door");
                nav->getClose(goal[0], goal[1], goal[2], 120000);

                hri->say("I have arrived at the inspection point", 1);
                rclcpp::sleep_for(std::chrono::seconds(2));
                hri->say("Please tell me to continue to the exit", 3);
                state = SM_WAIT_FOR_COMMAND;
                break;

            case SM_WAIT_FOR_COMMAND:
                std::cout << "[SM_WAIT_FOR_COMMAND]" << std::endl;
                command = hri->lastRecognizedSpeech("test.json");

                if (command == "continue")
                    state = SM_NAVIGATE_TO_EXIT;

                break;

            case SM_NAVIGATE_TO_EXIT:
                std::cout << "[SM_NAVIGATE_TO_EXIT]" << std::endl;
                hri->say("I am going to the exit point", 3);
                rclcpp::sleep_for(std::chrono::milliseconds(500));

                goal = knowledge->getCoordinates("inspection_point");
                nav->getClose(goal[0], goal[1], goal[2], 180000);

                state = SM_FINAL;
                break;

            case SM_FINAL:
                std::cout << "[SM_FINAL]" << std::endl;
                hri->say("I have finished the test", 3);
                rclcpp::sleep_for(std::chrono::seconds(1));
                finished = true;
                break;
        }

        rclcpp::spin_some(node);
        loop.sleep();
    }

    rclcpp::shutdown();
    return 0;
}

