#ifndef _NAV_SIM_H_
#define _NAV_SIM_H_

#include <cmath>
#include <mutex>
#include <random>

#include <yaml-cpp/yaml.h>

#include <nav_sim/LandmarkInfo.h>
#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/PoseWithCovarianceStamped.h>
#include <geometry_msgs/Twist.h>
#include <geometry_msgs/TwistStamped.h>
#include <nav_msgs/Path.h>
#include <nav_msgs/Odometry.h>
#include <ros/ros.h>
#include <visualization_msgs/Marker.h>
#include <visualization_msgs/MarkerArray.h>
#include <tf2/LinearMath/Quaternion.h>
#include <tf2_geometry_msgs/tf2_geometry_msgs.h>
#include <tf2_ros/static_transform_broadcaster.h>
#include <tf2_ros/transform_broadcaster.h>

struct State
{
  double x_;
  double y_;
  double yaw_;
  State() : x_(0.0), y_(0.0), yaw_(0.0) {}
  State(double x, double y, double yaw) : x_(x), y_(y), yaw_(yaw) {}
};

struct Landmark
{
  std::string landmark_id_;
  double x_;
  double y_;
  double yaw_;
  Landmark() : x_(0.0), y_(0.0), landmark_id_("") {}
  Landmark(double x, double y, std::string landmark_id) : x_(x), y_(y), landmark_id_(landmark_id) {}
};

class NavSim
{
public:
  NavSim() : nh_(), pnh_("~"), v_(0.0), w_(0.0) { initialize(); }
  ~NavSim() {}

  template <typename PoseType>
  geometry_msgs::PoseStamped convertToPose(PoseType state);
  tf2::Transform convertToTransform(geometry_msgs::PoseStamped pose);
  std::vector<Landmark> parseYaml(const std::string yaml);
  void initialize();
  void simTransferError(State & state, double time_interval);
  void publishPoseToTransform(geometry_msgs::PoseStamped pose);
  void velocityFilter(double & target_v, double & target_w);
  void observation(std::vector<Landmark> landmark_queue);
  inline double getExponentialDistribution(double parameter)
  {
    std::random_device seed;
    std::default_random_engine engine(seed());
    std::exponential_distribution<> exponential(parameter);
    return exponential(engine);
  }
  inline double getGaussDistribution(double mean, double std)
  {
    std::random_device seed;
    std::default_random_engine engine(seed());
    std::normal_distribution<> gauss(mean, std);
    return gauss(engine);
  }

  inline double normalizeDegree(const double degree)
  {
    double normalize_deg = std::fmod((degree+180.0), 360.0) - 180.0;
    if(normalize_deg < -180.0) normalize_deg += 360.0;
    return normalize_deg;
  }
  inline void callbackCmdVel(const geometry_msgs::Twist & msg)
  {
    cmd_vel_ = msg;
  }
  void callbackInitialpose(const geometry_msgs::PoseWithCovarianceStamped & msg);
  void timerCallback(const ros::TimerEvent & e);

  void clearMarker();

private:
  State current_state_;
  State ground_truth_;

  ros::NodeHandle nh_;
  ros::NodeHandle pnh_;
  ros::Timer timer_;

  double distance_until_noise_;

  double period_;
  double limit_view_angle_;
  double error_coeff_;
  double previous_time_;
  double v_, w_;

  std::string config_;

  std::vector<Landmark> landmark_pose_list_;

  nav_sim::LandmarkInfo landmark_queue_;

  geometry_msgs::Twist cmd_vel_;
  geometry_msgs::PoseStamped current_pose_;
  geometry_msgs::PoseStamped ground_truth_pose_;

  ros::Publisher current_velocity_publisher_;
  ros::Publisher ground_truth_publisher_;
  ros::Publisher odometry_publisher_;
  ros::Publisher observation_publisher_;

  ros::Publisher landmark_info_pub_;
  ros::Publisher currnet_pose_publisher_;
  ros::Publisher path_pub_;
  ros::Subscriber cmd_vel_sub_;
  ros::Subscriber initialpose_sub_;
};

#endif
