// coordinator_action_client: 
// wsn, September, 2016

#include<ros/ros.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include<coordinator/ManipTaskAction.h>
#include <object_manipulation_properties/object_manipulation_properties.h>


bool g_goal_done = true;
int g_ntasks_done = 0;
int g_callback_status = coordinator::ManipTaskResult::PENDING;
//bool g_goal_active=false;

void doneCb(const actionlib::SimpleClientGoalState& state,
        const coordinator::ManipTaskResultConstPtr& result) {
    ROS_INFO(" doneCb: server responded with state [%s]", state.toString().c_str());
    g_goal_done = true;
    g_ntasks_done++;
    g_callback_status = result->manip_return_code;
    ROS_INFO("return status is %d ", g_callback_status);
    //int diff = result->output - result->goal_stamp;
    //ROS_INFO("got result output = %d; goal_stamp = %d; diff = %d", result->output, result->goal_stamp, diff);
}

void feedbackCb(const coordinator::ManipTaskFeedbackConstPtr& fdbk_msg) {
    ROS_INFO("feedback status = %d", fdbk_msg->feedback_status);
    //g_fdbk = fdbk_msg->feedback_status; //make status available to "main()"
}

// Called once when the goal becomes active; not necessary, but possibly useful for diagnostics

void activeCb() {
    ROS_INFO("Goal just went active");
}

int main(int argc, char** argv) {
    ros::init(argc, argv, "task_client_node"); // name this node 
    ros::NodeHandle nh; //standard ros node handle    

    int g_count = 0;
    coordinator::ManipTaskGoal goal;

    goal.dropoff_frame.header.frame_id = "torso";
    goal.dropoff_frame.pose.position.x = 0.5;
    goal.dropoff_frame.pose.position.y = -0.35;
    goal.dropoff_frame.pose.position.z = -0.145;
    goal.dropoff_frame.pose.orientation.x = 0;
    goal.dropoff_frame.pose.orientation.y = 0;
    goal.dropoff_frame.pose.orientation.z = 0.842;
    goal.dropoff_frame.pose.orientation.w = 0.54;
    goal.dropoff_frame.header.stamp = ros::Time::now();
    goal.pickup_frame = goal.dropoff_frame;
    goal.pickup_frame.pose.position.y = -0.5; //drop block here

    // use the name of our server, which is: example_action (named in example_action_server.cpp)
    // the "true" argument says that we want our new client to run as a separate thread (a good idea)
    actionlib::SimpleActionClient<coordinator::ManipTaskAction> action_client("manip_task_action_service", true);

    // attempt to connect to the server:
    ROS_INFO("waiting for server: ");
    bool server_exists = false;
    while ((!server_exists)&&(ros::ok())) {
        server_exists = action_client.waitForServer(ros::Duration(0.5)); // 
        ros::spinOnce();
        ros::Duration(0.5).sleep();
        ROS_INFO("retrying...");
    }

    ROS_INFO("connected to action server"); // if here, then we connected to the server;


    ROS_INFO("sending a goal: move to pre-pose");
    g_goal_done = false;
    goal.action_code = coordinator::ManipTaskGoal::MOVE_TO_PRE_POSE;
    action_client.sendGoal(goal, &doneCb, &activeCb, &feedbackCb);
    while (!g_goal_done) {
        ros::Duration(0.1).sleep();
    }

    //now send a manipulation code, including vision, grasp and drop-off
    g_goal_done = false;
    goal.object_code = TOY_BLOCK_ID; // from object_manipulation_properties; //coordinator::ManipTaskGoal::TOY_BLOCK;
    goal.action_code = coordinator::ManipTaskGoal::MANIP_OBJECT;
    //goal.perception_source= coordinator::ManipTaskGoal::BLIND_MANIP;
    goal.perception_source = coordinator::ManipTaskGoal::PCL_VISION;

    action_client.sendGoal(goal, &doneCb, &activeCb, &feedbackCb);

    while (!g_goal_done) {
        ros::Duration(0.1).sleep();
    }
    ROS_INFO("callback reports goal is done; quitting");

    return 0;
}

