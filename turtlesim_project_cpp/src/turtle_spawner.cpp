#include "rclcpp/rclcpp.hpp"
#include <cmath>
#include "turtlesim/srv/spawn.hpp"

using namespace std::placeholders;
using namespace std::chrono_literals;

class TurtleSpawnerNode : public rclcpp::Node
{
public:
    TurtleSpawnerNode() : Node("turtle_spawner"), turtle_counter_(0)
    {
        turtle_name_prefix_ = "turtle";
        spawn_client_ = this->create_client<turtlesim::srv::Spawn>("spawn");
        spawn_turtle_timer_ = this->create_wall_timer(
            2s, std::bind(&TurtleSpawnerNode::spawnNewTurtle, this));
    }

private:

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

        spawn_client_->async_send_request(
            request, std::bind(&TurtleSpawnerNode::callbackCallSpawnTurtleService, this, _1));  
    }

    void callbackCallSpawnTurtleService(rclcpp::Client<turtlesim::srv::Spawn>::SharedFuture future)
    {
        auto response = future.get();
        if (response->name != "")
        {
            RCLCPP_INFO(this->get_logger(), "Turtle %s is now alive.", response->name.c_str());
        }
    }


    std::string turtle_name_prefix_;
    int turtle_counter_;

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