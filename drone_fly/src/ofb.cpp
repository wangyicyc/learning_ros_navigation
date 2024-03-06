#include <geometry_msgs/PoseStamped.h>
#include <geometry_msgs/Twist.h>

//mavros_msgs功能包中包含操作mavros包中服务和主题所需要的自定义消息文件（所以前面在创建功能包的时候就加入了那两个功能包）
#include <mavros_msgs/CommandBool.h>
#include <mavros_msgs/SetMode.h>
#include <mavros_msgs/State.h>

#include <nav_msgs/Odometry.h>
#include <ros/ros.h>
#include <sensor_msgs/LaserScan.h>

#include <nav_msgs/Path.h>
#include <std_msgs/String.h>
#include <tf/tf.h>
#include <Eigen/Dense>
#include <Eigen/Eigen>
#include <Eigen/Eigenvalues>
#include <Eigen/Geometry>
#include <cmath>

geometry_msgs::PoseStamped pos_drone;       //记录飞机position
geometry_msgs::Twist vel;
std_msgs::String state;
geometry_msgs::PoseStamped pose;

double x,y,z;
double r,p,yaw;
double fly_height=1.2;

struct Pose
{
    double x;
    double y;
}waypoint[4];

enum MODE
{   
    READY,
    RTL,//AUTO.RTL
    OFFBOARD,
    TAKEOFF,//AUTO.TAKEOFF
    ARM,//AUTO.RTL
    LAND,//AUTO.LAND
    FIRST,
    SECOND,
}fly_mode;


void pos_cb(const nav_msgs::Odometry::ConstPtr& odom_3d) 
{
    pos_drone.pose.position.x = odom_3d->pose.pose.position.x;
    pos_drone.pose.position.y = odom_3d->pose.pose.position.y;
    pos_drone.pose.position.z = odom_3d->pose.pose.position.z;
    pos_drone.pose.orientation = odom_3d->pose.pose.orientation;
    
    //x,y,z用来存放odom的position，后面用于设置随机期望点（pose），连接飞控
    x = pos_drone.pose.position.x;
    y = pos_drone.pose.position.y;
    z = pos_drone.pose.position.z;
    tf::Quaternion RQ2;
    tf::quaternionMsgToTF(pos_drone.pose.orientation, RQ2);
    tf::Matrix3x3(RQ2).getRPY(r, p, yaw);

    //std::cout<<"position x:"<<x<<",y:"<<y<<",z:"<<z<<std::endl;
}

bool isget(Pose goal,int dis=0.1)
{
    if (fabs(goal.x - x) < dis && fabs(goal.y - y) < dis)
        return true;
    else
        return false;
}

int main(int argc,char **argv)
{   


    waypoint[0].x=0;
    waypoint[0].y=0;
    //目标点
    waypoint[1].x=1.5;
    waypoint[1].y=6;
    waypoint[2].x=6;
    waypoint[2].y=2;

    ros::init(argc,argv,"drone_demo");
    ros::NodeHandle n;

    //Publisher
    ros::Publisher cmd_pub=n.advertise<std_msgs::String>("/xtdrone/iris_0/cmd",2);
    ros::Publisher local_pos_pub = n.advertise<geometry_msgs::PoseStamped>("iris_0/mavros/setpoint_position/local", 10);
    ros::Publisher local_vel_pub = n.advertise<geometry_msgs::Twist>("/xtdrone/iris_0/cmd_vel_flu", 1);   
    //Subscriber
    ros::Subscriber position_sub = n.subscribe<nav_msgs::Odometry>("iris_0/mavros/local_position/odom", 10, pos_cb);
    
    //发布目标点
    ros::Rate rate(5);
    

    pose.pose.position.x = x;
    pose.pose.position.y = y;
    pose.pose.position.z = fly_height;

    pose.pose.orientation.w = 1;
    pose.pose.orientation.x = 0;
    pose.pose.orientation.y = 0;
    pose.pose.orientation.z = 0;

    vel.linear.x = 0;
    vel.linear.y = 0;
    vel.linear.z = 0;
    vel.angular.x = 0;
    vel.angular.y = 0;
    vel.angular.z = 0;

    fly_mode=READY;
    state.data="OFFBOARD";
    cmd_pub.publish(state);
    ros::spinOnce();

    ros::Time last_request = ros::Time::now();  //记录最后一次请求的时间戳

    while (ros::ok())
    {
        switch (fly_mode)
        {
        case READY:
            //该语句的作用：若不加此句，那么可能因为程序运行速度问题，短时间内进行了多次循环，从而“跳过”了某些case语句
            if(state.data!="AUTO.TAKEOFF" && (ros::Time::now() - last_request > ros::Duration(0.5)))
            {
                fly_mode=TAKEOFF;
                state.data="AUTO.TAKEOFF";
                last_request = ros::Time::now();
            }
            
            break;

        case TAKEOFF:
            if(state.data!="ARM" && (ros::Time::now() - last_request > ros::Duration(0.5)))
            {   
                fly_mode=OFFBOARD;
                state.data="ARM";
                std::cout<<"TAKEOFF NOW !!"<<std::endl;
                last_request = ros::Time::now();
            }
            
            break;


        case OFFBOARD:

            if (std::fabs(z - fly_height) < 0.1)
            {   
                std::cout<<"GO FIRST NOW !!"<<std::endl;
                state.data="OFFBOARD";
                fly_mode=FIRST;
                last_request = ros::Time::now();
            }
            break;
            
        case FIRST:
            pose.pose.position.x=waypoint[1].x;
            pose.pose.position.y=waypoint[1].y;
            pose.pose.position.z=fly_height;
            local_pos_pub.publish(pose);

            if (isget(waypoint[1], 0.1)) 
            {
                std::cout<<"GO SECOND NOW!!"<<std::endl;
                fly_mode = SECOND;
                last_request = ros::Time::now();
            }

            /*
            
            */
            /*
            vel.linear.x = 2;     
            vel.linear.y = 2;
            local_vel_pub.publish(vel); 
            */


            
            /*if (ros::Time::now()-last_request > ros::Duration(5))
            {
                fly_mode=SECOND;
                ROS_INFO("GO SECOND NOW!!");
                last_request=ros::Time::now();
            }*/

            
            
            break;

        case SECOND:
            pose.pose.position.x=waypoint[2].x;
            pose.pose.position.y=waypoint[2].y;
            pose.pose.position.z=fly_height;


        default:
            break;
        }

      
        cmd_pub.publish(state);
        ros::spinOnce();
        rate.sleep();
    }
    
    return 0;
}
