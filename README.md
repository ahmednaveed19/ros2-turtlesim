# ROS 2 Project: Turtlesim "Catch Them All"

## Project Overview
The objective of this project is to build an interactive target-tracking simulation using ROS 2 and C++, leveraging the standard **Turtlesim** package as a visual environment. The core application features a primary "main" turtle that autonomously tracks, pursues, and catches target turtles spawned dynamically across the screen.

---

## Prerequisites & Dependencies
* **Operating System:** Ubuntu 24.04
* **ROS 2 Distribution:** ROS 2 Jazzy

## Running the Project
1. **Build the Workspace**  
   Open a terminal, navigate to your ROS 2 workspace root directory, and build the project:
   ```bash
   colcon build
   ```
2. **Source the Workspace**  
   Open a new terminal and source your workspace overlay: 
   ```bash
   source install/setup.bash
   ```
   **Note:** Adding `source install/setup.bash` command ensures that ROS 2 can locate your built packages and custom interfaces when running in the active terminal session.

3. **Execute the Launch File**  
   Run the launch command to start the application:
   ```bash
   ros2 launch my_robot_bringup turtlesim_catch_them_all.launch.xml
   ```

---

## System Architecture & Nodes
The project architecture relies on three distinct nodes, typically organized within a dedicated package (`turtlesim_project_cpp`):

1. **`turtlesim` (from the `turtlesim` package):** Serves as the core visual simulation environment.
2. **`turtle_controller` (Custom Node):** Manages the master turtle (`turtle1`). It runs a high-rate control loop utilizing a proportional (P) controller, subscribes to pose and active turtle data, publishes velocity commands, and communicates catch events.
3. **`turtle_spawner` (Custom Node):** Handles entity lifecycle management. It periodically spawns new turtles at random coordinates, tracks active entities, and interfaces with the simulator's native services.

---

## Communication Interface

### Topics & Messages
* **/turtle1/pose:** Subscribed to by `turtle_controller` to track the master turtle's current position and orientation.
* **/turtle1/cmd_vel:** Published to by `turtle_controller` to steer the master turtle.
* **/alive_turtles:** Published by `turtle_spawner` and subscribed to by `turtle_controller` to broadcast the list of active turtles and their coordinates. Requires custom message definitions (`Turtle.msg` and `TurtleArray.msg`).

### Services
* **/spawn & /kill:** Native services provided by `turtlesim`. The `turtle_spawner` calls `/spawn` to introduce new targets and `/kill` to clear caught targets.
* **/catch_turtle:** A custom service (`CatchTurtle.srv`) hosted by `turtle_spawner` and called by `turtle_controller` whenever a target turtle is successfully reached.

---

## Step-by-Step Implementation Roadmap

1. **Controller Setup & Kinematics**  
   Create the `turtle_controller` node, subscribe to `/turtle1/pose`, and implement a control loop using basic geometry and math to navigate toward an arbitrary target coordinate by publishing to `/turtle1/cmd_vel`.
   
2. **Spawner Implementation**  
   Create the `turtle_spawner` node utilizing a timer to trigger the native `/spawn` service at a set frequency, generating new turtles at random coordinates ($x, y \in [0.0, 11.0]$).

3. **State Tracking & Topic Publishing**  
   Maintain an internal array of alive turtles inside `turtle_spawner` and broadcast it via the `/alive_turtles` topic. Update `turtle_controller` to subscribe to this topic and target the first entity listed in the array.

4. **Catch Logic & Service Integration**  
   Implement the `/catch_turtle` service on `turtle_spawner`. Configure `turtle_controller` to invoke this service upon reaching a target, triggering `turtle_spawner` to call the native `/kill` service, purge the target from the active array, and publish the updated list.

5. **Algorithmic Enhancement**  
   Upgrade the target-selection logic in `turtle_controller` so that the master turtle dynamically tracks and pursues the **closest** available turtle rather than defaulting to the first index.

6. **Parameterization and Launch Configuration**  
   Scale the application by integrating runtime parameters and configuration files within a `my_robot_bringup` using an `.xml` format for launch files and a `.yaml` format for parameter files:
   * **/turtle_controller parameters:** `catch_closest_turtle_first`
   * **/turtle_spawner parameters:** `spawn_frequency`, `turtle_name_prefix`  
