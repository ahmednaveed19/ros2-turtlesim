#include "rclcpp/rclcpp.hpp"
#include <cmath>
#include "turtlesim/srv/spawn.hpp"
#include "my_robot_interfaces/msg/turtle.hpp"
#include "my_robot_interfaces/msg/turtle_array.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

class TurtleSpawnerNode : public rclcpp::Node
{
public:
    TurtleSpawnerNode() : Node("turtle_spawner"), turtle_counter_(0)
    {
        turtle_name_prefix_ = "turtle";
        alive_turtles_publisher_ =
            this->create_publisher<my_robot_interfaces::msg::TurtleArray>("alive_turtles", 10);
        spawn_client_ = this->create_client<turtlesim::srv::Spawn>("spawn");
        spawn_turtle_timer_ = this->create_wall_timer(
            2s, std::bind(&TurtleSpawnerNode::spawnNewTurtle, this));
    }

private:
    void publishAliveTurtles()
    {
        auto msg = my_robot_interfaces::msg::TurtleArray();
        msg.turtles = alive_turtles_;
        alive_turtles_publisher_->publish(msg);
    }

    // returns random double number in range [0.0, 1.0)
    double randomDouble()
    {
        return double(std::rand()) / (double(RAND_MAX) + 1.0);
    }

    void spawnNewTurtle()
    {
        turtle_counter_++;
        auto name = turtle_name_prefix_ + std::to_string(turtle_counter_);
        double x = randomDouble() * 10.0;
        double y = randomDouble() * 10.0;
        double theta = randomDouble() * 2 * M_PI;

        callSpawnTurtleService(name, x, y, theta);
    }

    void callSpawnTurtleService(std::string turtle_name, double x, double y, double theta)
    {
        while (!spawn_client_->wait_for_service(1s))
        {
            RCLCPP_WARN(this->get_logger(), "Waiting for Service Server to be up...");
        }

        auto request = std::make_shared<turtlesim::srv::Spawn::Request>();
        request->name = turtle_name;
        request->x = x;
        request->y = y;
        request->theta = theta;

        turtle_to_save_ = my_robot_interfaces::msg::Turtle();
        turtle_to_save_.name = turtle_name;
        turtle_to_save_.x = x;
        turtle_to_save_.y = y;
        turtle_to_save_.theta = theta;

        spawn_client_->async_send_request(
            request, std::bind(&TurtleSpawnerNode::callbackCallSpawnTurtleService, this, _1));  
    }

    void callbackCallSpawnTurtleService(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
    {
        auto response = future.get();
        if (response->name != "")
        {
            RCLCPP_INFO(this->get_logger(), "Turtle %s is now alive.", response->name.c_str());
            auto new_turtle = my_robot_interfaces::msg::Turtle();
            new_turtle.name = turtle_to_save_.name;
            new_turtle.x = turtle_to_save_.x;
            new_turtle.y = turtle_to_save_.y;
            new_turtle.theta = turtle_to_save_.theta;
            alive_turtles_.push_back(new_turtle);
            publishAliveTurtles();

        }
    }


    std::string turtle_name_prefix_;
    int turtle_counter_;
    my_robot_interfaces::msg::Turtle turtle_to_save_;
    std::vector<my_robot_interfaces::msg::Turtle> alive_turtles_;

    rclcpp::Publisher<my_robot_interfaces::msg::TurtleArray>::SharedPtr alive_turtles_publisher_;
    rclcpp::Client<turtlesim::srv::Spawn>::SharedPtr spawn_client_;
    rclcpp::TimerBase::SharedPtr spawn_turtle_timer_;

};

int main(int argc, char **argv)
{
    rclcpp::init(argc, argv);
    auto node = std::make_shared<TurtleSpawnerNode>();
    rclcpp::spin(node);
    rclcpp::shutdown();
    return 0;
}