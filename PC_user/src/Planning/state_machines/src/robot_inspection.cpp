#include <chrono>
#include <iostream>
#include <vector>
#include <string>
#include <memory>

#include "rclcpp/rclcpp.hpp"
#include "festino_tools/FestinoHRI.h"
#include "festino_tools/FestinoNavigation.h"
#include "festino_tools/FestinoKnowledge.h"

enum SMState
{
    SM_INIT,
    SM_NAVIGATE_TO_INSPECTION,
    SM_WAIT_FOR_COMMAND,
    SM_NAVIGATE_TO_EXIT,
    SM_FINAL_STATE
};

int main(int argc, char **argv)
{
    std::cout << "INITIALIZING ROBOT INSPECTION..." << std::endl;

    rclcpp::init(argc, argv);
    auto node = rclcpp::Node::make_shared("robot_inspection");

    FestinoHRI::setNodeHandle(node);
    FestinoNavigation::setNodeHandle(node);
    FestinoKnowledge::setNodeHandle(node);

    rclcpp::Rate loop(10);

    SMState state = SM_INIT;
    std::vector<float> goal_vec(3);
    std::string recog;

    bool success = false;

    FestinoHRI::say(" ", 1);

    while (rclcpp::ok() && !success)
    {
        switch (state)
        {
            case SM_INIT:
                std::cout << "[SM_INIT] Robot ready for inspection" << std::endl;
                FestinoHRI::say("I am ready for robot inspection", 3);
                state = SM_NAVIGATE_TO_INSPECTION;
                break;

            case SM_NAVIGATE_TO_INSPECTION:
                std::cout << "[SM_NAVIGATE_TO_INSPECTION] Moving to inspection point" << std::endl;
                FestinoHRI::say("I am going to the inspection point", 3);
                rclcpp::sleep_for(std::chrono::seconds(3));

                goal_vec = FestinoKnowledge::CoordenatesLocSrv("entrance_door");
                std::cout << "Inspection point coordinates: x=" << goal_vec[0]
                          << ", y=" << goal_vec[1]
                          << ", a=" << goal_vec[2] << std::endl;

                if (!FestinoNavigation::getClose(goal_vec[0], goal_vec[1], goal_vec[2], 120000))
                    std::cout << "Cannot move to inspection point" << std::endl;

                FestinoHRI::say("I have arrived at the inspection point", 1);
                rclcpp::sleep_for(std::chrono::seconds(2));
                FestinoHRI::say("Please tell me to continue to the exit", 3);
                rclcpp::sleep_for(std::chrono::seconds(2));

                state = SM_WAIT_FOR_COMMAND;
                break;

            case SM_WAIT_FOR_COMMAND:
                std::cout << "[SM_WAIT_FOR_COMMAND] Waiting for command" << std::endl;
                recog = FestinoHRI::lastRecogSpeech("test.json");

                if (recog != "continue")
                {
                    std::cout << "Heard: " << recog << ", waiting..." << std::endl;
                }
                else
                {
                    std::cout << "Heard: " << recog << ", proceeding to exit" << std::endl;
                    state = SM_NAVIGATE_TO_EXIT;
                }
                break;

            case SM_NAVIGATE_TO_EXIT:
                std::cout << "[SM_NAVIGATE_TO_EXIT] Moving to exit point" << std::endl;
                FestinoHRI::say("I am going to the exit point", 3);
                rclcpp::sleep_for(std::chrono::milliseconds(500));

                goal_vec = FestinoKnowledge::CoordenatesLocSrv("inspection_point");
                std::cout << "Exit point coordinates: x=" << goal_vec[0]
                          << ", y=" << goal_vec[1]
                          << ", a=" << goal_vec[2] << std::endl;

                if (!FestinoNavigation::getClose(goal_vec[0], goal_vec[1], goal_vec[2], 180000))
                {
                    FestinoHRI::say("Cannot move to exit point", 3);
                    break;
                }

                state = SM_FINAL_STATE;
                break;

            case SM_FINAL_STATE:
                std::cout << "[SM_FINAL_STATE] Test finished" << std::endl;
                FestinoHRI::say("I have finished the test", 3);
                rclcpp::sleep_for(std::chrono::seconds(1));
                success = true;
                break;

            default:
                break;
        }

        rclcpp::spin_some(node);
        loop.sleep();
    }

    rclcpp::shutdown();
    return 0;
}
