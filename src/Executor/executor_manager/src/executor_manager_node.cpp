#include <chrono>
#include <functional>
#include <memory>
#include <mutex>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "rclcpp_action/rclcpp_action.hpp"

#include "executor_interfaces/action/execute_task.hpp"
#include "executor_interfaces/msg/task_step.hpp"
#include "executor_interfaces/msg/task_feedback.hpp"
#include "executor_interfaces/msg/task_result.hpp"

using namespace std::chrono_literals;

class TaskManagerNode : public rclcpp::Node
{
public:
  using ExecuteTask = executor_interfaces::action::ExecuteTask;
  using GoalHandleExecuteTask = rclcpp_action::ClientGoalHandle<ExecuteTask>;

  using TaskStep     = executor_interfaces::msg::TaskStep;
  using TaskFeedback = executor_interfaces::msg::TaskFeedback;
  using TaskResult   = executor_interfaces::msg::TaskResult;

  explicit TaskManagerNode(const rclcpp::NodeOptions & options = rclcpp::NodeOptions())
  : Node("task_manager_node", options)
  {
    action_client_ = rclcpp_action::create_client<ExecuteTask>(this, "/executor/executor_backend/execute_task");

    pub_feedback_ = this->create_publisher<TaskFeedback>("/client/feedback", 10);
    pub_result_   = this->create_publisher<TaskResult>("/client/result", 10);

    sub_step_ = this->create_subscription<TaskStep>(
      "/task_step",
      10,
      std::bind(&TaskManagerNode::on_task_step, this, std::placeholders::_1)
    );

    RCLCPP_INFO(get_logger(), "✅ task_manager_node listo.");
    RCLCPP_INFO(get_logger(), "🎯 Action client -> /execute_task");
    RCLCPP_INFO(get_logger(), "📩 Subscribed to: /task_step (TaskStep)");
    RCLCPP_INFO(get_logger(), "📡 Extra topics: /client/feedback (TaskFeedback) y /client/result (TaskResult)");
  }

private:
  // ---- ROS entities ----
  rclcpp_action::Client<ExecuteTask>::SharedPtr action_client_;

  rclcpp::Publisher<TaskFeedback>::SharedPtr pub_feedback_;
  rclcpp::Publisher<TaskResult>::SharedPtr   pub_result_;

  rclcpp::Subscription<TaskStep>::SharedPtr sub_step_;

  // (Opcional) evita mandar 2 goals a la vez si no quieres concurrencia
  std::mutex send_mutex_;

  // ---- Callbacks ----
  void on_task_step(const TaskStep::SharedPtr msg)
  {
    if (!msg) return;

    // Copia local (seguro ante concurrencia)
    TaskStep step = *msg;

    RCLCPP_INFO(get_logger(),
                "📩 [CLIENT] /task_step recibido -> step_id=%d action='%s'",
                step.step_id, step.action.c_str());

    send_action_goal(step);
  }

  void send_action_goal(const TaskStep & step_msg)
  {
    std::lock_guard<std::mutex> lk(send_mutex_);

    const auto step_id = step_msg.step_id;
    const auto action  = step_msg.action;

    if (!action_client_->wait_for_action_server(2s)) {
      RCLCPP_ERROR(get_logger(), "❌ Action Server 'execute_task' no disponible.");

      TaskResult out;
      out.step_id = step_id;
      out.status  = TaskResult::FAILURE;   // si no existe en tu msg, ajusta aquí
      out.message = "Action server not available";
      pub_result_->publish(out);
      return;
    }

    ExecuteTask::Goal goal;
    goal.task = step_msg;

    RCLCPP_INFO(get_logger(),
                "📤 [CLIENT] Enviando GOAL -> step_id=%d action='%s'",
                step_id, action.c_str());

    rclcpp_action::Client<ExecuteTask>::SendGoalOptions options;

    options.goal_response_callback =
      [this, step_id, action](GoalHandleExecuteTask::SharedPtr goal_handle)
      {
        if (!goal_handle) {
          RCLCPP_ERROR(get_logger(),
                       "⛔ [CLIENT] Goal RECHAZADO -> step_id=%d action='%s'",
                       step_id, action.c_str());

          TaskResult out;
          out.step_id = step_id;
          out.status  = TaskResult::FAILURE;
          out.message = "Goal rejected by server";
          pub_result_->publish(out);
          return;
        }

        RCLCPP_INFO(get_logger(),
                    "✅ [CLIENT] Goal ACEPTADO -> step_id=%d action='%s'",
                    step_id, action.c_str());
      };

    options.feedback_callback =
      [this](GoalHandleExecuteTask::SharedPtr /*gh*/,
             const std::shared_ptr<const ExecuteTask::Feedback> fb)
      {
        const auto & f = fb->feedback;  // TaskFeedback

        RCLCPP_INFO(get_logger(),
                    "📥 [CLIENT] FEEDBACK -> step_id=%d message='%s'",
                    f.step_id, f.message.c_str());

        pub_feedback_->publish(f);
      };

    options.result_callback =
      [this](const GoalHandleExecuteTask::WrappedResult & wr)
      {
        const auto & r = wr.result->result;  // TaskResult

        const char * code_txt = "UNKNOWN";
        switch (wr.code) {
          case rclcpp_action::ResultCode::SUCCEEDED: code_txt = "SUCCEEDED"; break;
          case rclcpp_action::ResultCode::ABORTED:   code_txt = "ABORTED";   break;
          case rclcpp_action::ResultCode::CANCELED:  code_txt = "CANCELED";  break;
          default: break;
        }

        RCLCPP_INFO(get_logger(),
                    "📦 [CLIENT] RESULT (%s) -> step_id=%d status=%d message='%s'",
                    code_txt, r.step_id, r.status, r.message.c_str());

        pub_result_->publish(r);
      };

    action_client_->async_send_goal(goal, options);
  }
};

int main(int argc, char ** argv)
{
  rclcpp::init(argc, argv);
  auto node = std::make_shared<TaskManagerNode>();
  rclcpp::spin(node);
  rclcpp::shutdown();
  return 0;
}
