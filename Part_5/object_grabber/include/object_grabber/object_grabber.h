//define a class to encapsulate some of the tedium of populating and sending goals,
// and interpreting responses
#ifndef BAXTER_OBJECT_GRABBER_H
#define	BAXTER_OBJECT_GRABBER_H
#include <ros/ros.h>
#include <actionlib/server/simple_action_server.h>
#include <actionlib/client/simple_action_client.h>
#include <actionlib/client/terminal_state.h>
#include <cartesian_planner/baxter_cart_moveAction.h>
#include <cartesian_planner/baxter_arm_motion_commander.h>
#include <geometry_msgs/PoseStamped.h>
#include <object_grabber/object_grabberAction.h>
#include <Eigen/Eigen>
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <xform_utils/xform_utils.h>
#include <cartesian_planner/baxter_arm_motion_commander.h>
#include <simple_baxter_gripper_interface/simple_baxter_gripper_interface.h>
#include <baxter_fk_ik/baxter_kinematics.h> 
#include <std_msgs/Bool.h>
#include <object_manipulation_properties/object_manipulation_properties.h>
#include <tf/transform_listener.h>
double GRIPPER_TIMEOUT = 5.0; //max waiting time for gripper open/close

class ObjectGrabber {
private:
    ros::NodeHandle nh_;
    XformUtils xformUtils;
    ArmMotionCommander armMotionCommander;
    BaxterGripper baxterGripper;
    ObjectManipulationProperties objectManipulationProperties;

    //messages to send/receive cartesian goals / results:
    object_grabber::object_grabberGoal grab_goal_;
    object_grabber::object_grabberResult grab_result_;
    object_grabber::object_grabberFeedback grab_fdbk_;
    geometry_msgs::PoseStamped object_pose_stamped_;
    geometry_msgs::PoseStamped object_pose_stamped_wrt_torso_;
    geometry_msgs::PoseStamped des_flange_pose_stamped_;
    geometry_msgs::PoseStamped des_flange_pose_stamped_wrt_torso_;
    geometry_msgs::PoseStamped des_gripper_pose_stamped_wrt_torso_;
    int object_code_;
    //std_msgs::Bool gripper_open,gripper_close;

    double gripper_theta_;
    double z_depart_, L_approach_, dz_approach_offset_;
    double gripper_table_z_;
    Eigen::Vector3d gripper_b_des_;
    Eigen::Vector3d gripper_n_des_;
    Eigen::Vector3d gripper_t_des_;
    Eigen::Vector3d grasp_origin_, approach_origin_, depart_origin_;
    Eigen::Matrix3d R_gripper_vert_cyl_grasp_;
    Eigen::Affine3d a_gripper_start_, a_gripper_end_;
    Eigen::Affine3d a_gripper_approach_, a_gripper_depart_, a_gripper_grasp_;
    Eigen::Affine3d a_flange_approach_, a_flange_depart_, a_flange_grasp_;
    Eigen::Affine3d a_right_gripper_frame_wrt_flange_;
    Baxter_IK_solver baxter_IK_solver_; // instantiate an IK solver
    tf::TransformListener tfListener;

    actionlib::SimpleActionServer<object_grabber::object_grabberAction> object_grabber_as_;
    //action callback fnc
    void executeCB(const actionlib::SimpleActionServer<object_grabber::object_grabberAction>::GoalConstPtr& goal);

    int vertical_cylinder_power_grasp(geometry_msgs::PoseStamped object_pose);

    //cmd to approach an object along toolframe z-axis direction and grasp it
    //provide grasp pose, expressed as toolframe pose w/rt torso, approach distance, and
    // a test value for gripper closure to confirm successful grasp
    int grasp_approach_tool_z_axis(geometry_msgs::PoseStamped des_flange_grasp_pose,
            double approach_dist, double gripper_close_test_val);
    int dropoff_from_above(geometry_msgs::PoseStamped gripper_pose_wrt_torso, double approach_dist);
    int move_flange_to(geometry_msgs::PoseStamped flange_pose_wrt_torso);
    int fine_move_flange_to(geometry_msgs::PoseStamped des_flange_pose_wrt_torso);
    int jspace_move_to_pre_pose(void);
    int jspace_move_flange_to(geometry_msgs::PoseStamped des_flange_pose_wrt_torso);

    geometry_msgs::PoseStamped convert_pose_to_torso_frame(geometry_msgs::PoseStamped pose_stamped);

    //given object_id and given object's pose, compute corresponding right-arm toolflange pose for grasp
    //input an object poseStamped and object_id; return a corresponding right-arm toolflange poseStamped for grasp
    geometry_msgs::PoseStamped object_to_flange_grasp_transform(int object_id, geometry_msgs::PoseStamped object_pose);

    //specialized fncs: describes a grasp transform for a specific object (TOY_BLOCK)
    Eigen::Affine3d block_grasp_transform(Eigen::Affine3d block_affine);
    geometry_msgs::PoseStamped block_grasp_transform(geometry_msgs::PoseStamped block_pose);
    geometry_msgs::PoseStamped block_to_flange_grasp_transform(geometry_msgs::PoseStamped block_pose);

    Eigen::Affine3d grasp_transform_;
    double approach_dist_, gripper_test_val_;
    int object_id_;

public:

    ObjectGrabber(ros::NodeHandle* nodehandle); //define the body of the constructor outside of class definition
    //command open gripper, and wait for opening to exceed "open_val_test" (e.g. 95.0)
    int open_gripper(double open_val_test);
    //command close gripper, and wait for opening to be less than "close_val_test" (e.g. 90.0, for block)
    int close_gripper(double close_val_test);

    void set_right_tool_xform(Eigen::Affine3d xform) {
        a_right_gripper_frame_wrt_flange_ = xform;
    };

    ~ObjectGrabber(void) {
    }
    Eigen::Affine3d get_right_tool_transform(void);

};
#endif