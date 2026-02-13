#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <thread>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "executor_interfaces/action/execute_task.hpp"
#include "executor_interfaces/msg/task_feedback.hpp"   // <-- para publicar feedback como topic
#include "executor_interfaces/msg/task_result.hpp"   // <-- para publicar feedback/result como topic

using namespace std::chrono_literals;

class TaskNode : public rclcpp::Node
{
public:
  using MyAction = executor_interfaces::action::ExecuteTask;
  using GoalHandle = rclcpp_action::ServerGoalHandle<MyAction>;
  using TaskFeedbackMsg = executor_interfaces::msg::TaskFeedback;
  using TaskResultMsg = executor_interfaces::msg::TaskResult;

  explicit TaskNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("task_node", options)
  {
    using std::placeholders::_1;
    using std::placeholders::_2;

    // Publishers extra (solo para debug/observabilidad)
    pub_feedback_ = this->create_publisher<TaskFeedbackMsg>("/server/feedback", 10);
    pub_result_   = this->create_publisher<TaskResultMsg>("/server/result", 10);

    server_ = rclcpp_action::create_server<MyAction>(
      this,
      "execute_task",
      std::bind(&TaskNode::handle_goal, this, _1, _2),
      std::bind(&TaskNode::handle_cancel, this, _1),
      std::bind(&TaskNode::handle_accepted, this, _1)
    );

    RCLCPP_INFO(get_logger(), "✅ Action Server listo en: /execute_task");
    RCLCPP_INFO(get_logger(), "📡 Extra topics: /server/feedback y /server/result");
  }

private:
  rclcpp_action::Server<MyAction>::SharedPtr server_;

  rclcpp::Publisher<TaskFeedbackMsg>::SharedPtr pub_feedback_;
  rclcpp::Publisher<TaskResultMsg>::SharedPtr pub_result_;

  rclcpp_action::GoalResponse handle_goal(
    const rclcpp_action::GoalUUID & /*uuid*/,
    std::shared_ptr<const MyAction::Goal> goal)
  {
    RCLCPP_INFO(get_logger(), "📩 [SERVER] Recibí GOAL: '%s'", goal->task.action.c_str());
    return rclcpp_action::GoalResponse::ACCEPT_AND_EXECUTE;
  }

  rclcpp_action::CancelResponse handle_cancel(const std::shared_ptr<GoalHandle> goal_handle)
  {
    (void)goal_handle;
    RCLCPP_WARN(get_logger(), "🛑 [SERVER] Solicitud de CANCEL recibida -> ACEPTADA");
    return rclcpp_action::CancelResponse::ACCEPT;
  }

  void handle_accepted(const std::shared_ptr<GoalHandle> goal_handle)
  {
    std::thread{std::bind(&TaskNode::execute, this, std::placeholders::_1), goal_handle}.detach();
  }

  void publish_feedback_topic(const MyAction::Goal & goal_msg, const std::string & text)
  {
    TaskFeedbackMsg msg;
    // Ajusta campos según tu .msg real:
    msg.step_id = goal_msg.task.step_id;
    msg.progress  = 0.0;  // si no existe, bórralo
    msg.message = text;

    pub_feedback_->publish(msg);
  }

  void publish_result_topic(const MyAction::Goal & goal_msg, const std::string & text, int32_t status)
  {
    TaskResultMsg msg;
    msg.step_id = goal_msg.task.step_id;
    msg.status  = status;   // SUCCESS / FAILURE, etc.
    msg.message = text;

    pub_result_->publish(msg);
  }

  void execute(const std::shared_ptr<GoalHandle> goal_handle)
  {
    const auto goal_msg = goal_handle->get_goal();

    RCLCPP_INFO(get_logger(), "🚀 [SERVER] Ejecutando goal: '%s'", goal_msg->task.action.c_str());

    auto feedback = std::make_shared<MyAction::Feedback>();
    auto result   = std::make_shared<MyAction::Result>();

    for (int i = 1; i <= 10; ++i) {
      if (goal_handle->is_canceling()) {
        result->result.step_id = goal_msg->task.step_id;
        result->result.status  = TaskResultMsg::FAILURE;   // o CANCELED si lo tienes
        result->result.message = "cancelled: " + goal_msg->task.action;

        RCLCPP_WARN(get_logger(), "🟡 [SERVER] Cancelando. RESULT: '%s'", result->result.message.c_str());

        // Publica también por topic
        publish_result_topic(*goal_msg, result->result.message, result->result.status);

        goal_handle->canceled(result);
        return;
      }

      feedback->feedback.step_id = goal_msg->task.step_id;
      feedback->feedback.message =
        "step " + std::to_string(i) + "/10 for goal: " + goal_msg->task.action;

      RCLCPP_INFO(get_logger(), "📣 [SERVER] FEEDBACK: '%s'", feedback->feedback.message.c_str());

      // Action feedback
      goal_handle->publish_feedback(feedback);

      // Topic feedback (extra)
      publish_feedback_topic(*goal_msg, feedback->feedback.message);

      std::this_thread::sleep_for(300ms);
    }

    result->result.step_id = goal_msg->task.step_id;
    result->result.status  = TaskResultMsg::SUCCESS;
    result->result.message = "done: " + goal_msg->task.action;

    RCLCPP_INFO(get_logger(), "✅ [SERVER] RESULT: '%s'", result->result.message.c_str());

    // Publica también por topic
    publish_result_topic(*goal_msg, result->result.message, result->result.status);

    goal_handle->succeed(result);
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<TaskNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
