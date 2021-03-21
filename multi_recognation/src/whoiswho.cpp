#include "recogfile.h"
#include "tts.h"
#include "ros/ros.h"
#include "std_msgs/String.h"
#include <geometry_msgs/Twist.h>
#include <sensor_msgs/LaserScan.h>
#include <stdlib.h>
#include <string.h>
#include <c++/5.4.0/iostream>
#include <c++/5.4.0/sstream>
#include <c++/5.4.0/string>
#include <c++/5.4.0/stdexcept>
#include <c++/5.4.0/vector>
#include <unistd.h>
#include "viewer.hpp"
#include <opencv2/opencv.hpp>
#include <ximgproc.hpp>
#include <seeta/FaceDetector2.h>
#include <seeta/FaceRecognizer.h>
#include <seeta/PointDetector2.h>
#include <seeta/Struct_cv.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <initializer_list>
using namespace std;
using namespace cv;

const size_t inWidth = 300;
const size_t inHeight = 300;
const float inScaleFactor = 0.007843f;
const float meanVal = 127.5;
#define n 2
seeta::FaceDetector2 FD("/home/robot/catkin_ws/include/SeetaFaceDetector2.0.ats");
seeta::FaceRecognizer2 FR("/home/robot/catkin_ws/include/SeetaFaceRecognizer2.0.ats");
seeta::PointDetector2 PD("/home/robot/catkin_ws/include/SeetaPointDetector2.0.pts5.ats");

string Names[]={"James","Alex","Ryan","John","Eric","Adam","Carter","Jack","David","Tyler","Lily","Mary","Anna","Zoe","Sara","Sofia","Faith","Julia","Paige","Jessica","person one","person two","person three"};
int name_order[3];
string filenames[4]={};
int name_num=0;
int counts=0;
int register_image(seeta::FaceDetector2 &FD, seeta::PointDetector2 &PD, seeta::FaceRecognizer2 &FR,const std::string &filename)
{
    cv::Mat mat = cv::imread(filename);
    if (mat.empty()) return -1;
    seeta::cv::ImageData image = mat;
    SeetaRect *face = FD.Detect(image);
    if (!face) return -1;
    SeetaPointF *points = PD.Detect(image, *face);
    if (!points) return -1;
    return FR.Register(image, points);  // Reture -1 if failed.
}
void people_recognition()
{
	std::string ns = K2_DEFAULT_NS;
	std::string topicColor = K2_TOPIC_QHD K2_TOPIC_IMAGE_COLOR K2_TOPIC_IMAGE_RECT;
	std::string topicDepth = K2_TOPIC_QHD K2_TOPIC_IMAGE_DEPTH K2_TOPIC_IMAGE_RECT;
	bool useExact = true;
	bool useCompressed = false;
	Receiver::Mode mode = Receiver::IMAGE;
	topicColor = "/" + ns + topicColor;
	topicDepth = "/" + ns + topicDepth;
	Receiver receiver(topicColor, topicDepth, useExact, useCompressed);
	receiver.start(mode);
	cv::Mat mat,depth;
	seeta::cv::ImageData image;
	SeetaRect *face;
	SeetaPointF *points;
	float similar=0;
	int num,index;
	tts("现在开始识别");
	tts("请所有人面向我保持不动");
	sleep(2);
	receiver.imageViewer(mat, depth, 1);
	image = mat;
	face= FD.Detect(image,&num);
	int i;
	for ( i = 1; i <= num; ++i,face++)
		{
			rectangle(mat,Rect(face->x,face->y,face->width,face->height),Scalar(255,0,0),3);
			points = PD.Detect(image, *face);
			index = FR.Recognize(image, points, &similar);
			cout<<"index:"<<index<<endl;
			cout<<"similar:"<<similar<<endl;
			if(similar<0.4)
			putText(mat, "Unknown", Point((int)( face->x + 0.5 * face->width ), 
					(int)( face->y + 0.5 * face->height )), FONT_HERSHEY_SIMPLEX, 1, Scalar(180, 105, 255), 3);
			else{
				if(index<=2)
						putText(mat, Names[name_order[index]], Point((int)( face->x + 0.5 * face->width ), 
					(int)( face->y + 0.5 * face->height )), FONT_HERSHEY_SIMPLEX, 1, Scalar(180, 105, 255), 3);
				else
						putText(mat, "Dui Yuan", Point((int)( face->x + 0.5 * face->width ), 
					(int)( face->y + 0.5 * face->height )), FONT_HERSHEY_SIMPLEX, 1, Scalar(180, 105, 255), 3);
				}
			}
		imwrite("//home//robot//zzu-sr_whoiswho_result//" + to_string(counts) + ".jpeg",mat);	
		tts("识别结束，准备离场");
}
unsigned int ques_find_or(string str_org, initializer_list<string> lst)
{
	unsigned int find_num = 0;
	for (auto i = lst.begin(); i != lst.end(); i++)
	{
		if (str_org.find(*i) != str_org.npos)
		{
			cout<<*i<<endl;
			return 1;
		}
	}
	return 0;
}

int find_name(string quest_str)
{
	if(ques_find_or(quest_str, {"ames","ance"}) )return 0;// "James";
	else if(ques_find_or(quest_str, {"lex"})) return 1;// "Alex";
	else if(ques_find_or(quest_str, {"yan","ri"})) return 2;// "Ryan";
	else if(ques_find_or(quest_str, {"ohn","rou"})) return 3;// "John";
	else if(ques_find_or(quest_str, {"ric","very","ake"}) )return 4;// "Eric";
	else if(ques_find_or(quest_str, {"dam"})) return 5;// "Adam";
	else if(ques_find_or(quest_str, {"ter","olor"}) )return 6;// "Carter";
	else if(ques_find_or(quest_str, {"ack","eck"})) return 7;// "Jack";
	else if(ques_find_or(quest_str, {"av"})) return 8;// "David";
	else if(ques_find_or(quest_str, {"ler","lor"})) return 9;// "Tyler";
	else if(ques_find_or(quest_str, {"ily"})) return 10;// "Lily";
	else if(ques_find_or(quest_str, {"ary"})) return 11;// "Mary";
	else if(ques_find_or(quest_str, {"na"})) return 12;// "Anna";
	else if(ques_find_or(quest_str, {"zo","ee"})) return 13;// "Zoe";
	else if(ques_find_or(quest_str, {"ra","eve"})) return 14;// "Sara";
	else if(ques_find_or(quest_str, {"fia","phie"})) return 15;// "Sofia";
	else if(ques_find_or(quest_str, {"aith","ace"}) )return 16;// "Faith";
	else if(ques_find_or(quest_str, {"lia"})) return 17;// "Julia";
	else if(ques_find_or(quest_str, {"aige","age"})) return 18;// "Paige";
	else if(ques_find_or(quest_str, {"essica"}) )return 19;// "Jessica";
	else return -1;
}

void face_memorize()
{
	std::string ns = K2_DEFAULT_NS;
	std::string topicColor = K2_TOPIC_QHD K2_TOPIC_IMAGE_COLOR K2_TOPIC_IMAGE_RECT;
	std::string topicDepth = K2_TOPIC_QHD K2_TOPIC_IMAGE_DEPTH K2_TOPIC_IMAGE_RECT;
	bool useExact = true;
	bool useCompressed = false;
	Receiver::Mode mode = Receiver::IMAGE;
	topicColor = "/" + ns + topicColor;
	topicDepth = "/" + ns + topicDepth;
	Receiver receiver(topicColor, topicDepth, useExact, useCompressed);
	receiver.start(mode);
	string name="";
	cv::Mat mat,depth;
	seeta::cv::ImageData image;
	SeetaRect *face;
	SeetaPointF *points;
	sleep(4);
	int recog_count=0;
	cout<<1<<endl;
	int name_index;
	tts("你好，请靠近我并大声说出你的名字");
		while(1)
	{
		cout<<"-----"<<endl;
		string get_result=recog();
		recog_count++;
		cout<<"-------"<<endl;
        if(get_result.empty()) 
        {
			if(recog_count<=5)
			{
			tts("请再说一遍");
			continue;
			}
		}
		name_index=find_name(get_result);
		if(name_index!=-1)
		{
			tts("我记住你的名字了");
			string str_all="你的名字是"+Names[name_index];
			name_order[name_num]=name_index;
			const char* p = str_all.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			break;
		}
		else if(recog_count>5)
		{
			name_index=7+name_num;
			name_order[name_num]=name_index;
			tts("对不起，我听不清你的名字");
			string str_all="暂时称呼你为"+Names[name_index];
			name_order[name_num]=name_index;
			const char* p = str_all.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			break;
			continue;
		}
		else
		{
			tts("请再说一遍");
			continue;
		}
	}
		tts("现在开始拍照");//正前方只有一个人
		//~ tts("请站在我正前方一米处，面朝我,半蹲或坐下，并保持不动");
		tts("请其他人不要在我面前，以免造成干扰");
		tts("我将在五秒后进行拍照");
		sleep(3);
		//位置待定
		while(1)
	{
		sleep(2);
		receiver.imageViewer(mat, depth, 1);
		image=mat;
		face = FD.Detect(image);

		if(face)
		{
			cout<<Names[name_index]<<endl;
			rectangle(mat,Rect(face->x,face->y,face->width,face->height),Scalar(255,0,0),1);
			filenames[name_num]="//home//robot//zzu-sr_whoiswho_result//" + to_string(name_num+100) + ".jpeg";
			imwrite("//home/robot//zzu-sr_whoiswho_result//" + to_string(name_num+100) + ".jpeg",mat);
			tts("我记住你的模样了，开始认识下一个人");
			break;
		}
		else
		{
			tts("识别失败，请调整位置重新拍照");
			continue;
		}
	}
	name_num++;
}
void turn()
{
		ros::NodeHandle nh;
		int rate=100;
		int flag=0;
		ros::Rate loop_rate(rate);
		move_base_msgs::MoveBaseGoal goal;
		double goal_angle,angle_duration;
		int ticks;
		geometry_msgs::Twist vel;
		ros::Publisher v_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 50, true);
		goal_angle = 3.14/180*120;
		vel.linear.x = 0.0;
		vel.angular.z = 0.2;
		angle_duration = goal_angle /vel.angular.z;
		ticks = int(angle_duration * rate);
		for(int j=0;j<ticks;j++)	
		{
		  v_pub.publish(vel);
		  loop_rate.sleep();
		}						
		vel.linear.x = 0;
		vel.angular.z = 0;	
		v_pub.publish(vel);
}
void move()
{
		ros::NodeHandle nh;
		int rate=100;
		int flag=0;
		ros::Rate loop_rate(rate);
		move_base_msgs::MoveBaseGoal goal;
		double goal_angle,angle_duration;
		int ticks;
		geometry_msgs::Twist vel;
		ros::Publisher v_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 50, true);
		float linear_speed,goal_distance,linear_duration;
		goal_distance = 1.2;
		vel.linear.x = 0.3;
		linear_duration = goal_distance /vel.linear.x;
		ticks = int(linear_duration * rate);
		vel.angular.z = 0;
		for(int j=0;j<ticks;j++)	
		{
		  v_pub.publish(vel);
		  loop_rate.sleep();
		}						
		vel.linear.x = 0;
		vel.angular.z = 0;	
		v_pub.publish(vel);
}

typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;

int main(int argc, char *argv[])
{
	/*========科大讯飞初始化=======================================================================*/

	const char* login_params= "appid = 5c7a2954, work_dir = .";//登录参数,appid与msc库绑定,请勿随意改动  5c7a2954
	MSPLogout(); 
	MSPLogin(NULL, NULL, login_params);//第一个参数是用户名，第二个参数是密码，第三个参数是登录参数，用户名和密码可在http://open.voicecloud.cn注册获取
	ros::init(argc, argv, "whoiswho");
	move_base_msgs::MoveBaseGoal goal;
	MoveBaseClient ac("move_base", true);   
	ac.waitForServer(ros::Duration(5.0));
	ROS_INFO("move_base action server come up");
	
	sleep(10);
	//进场
	move();
	
	goal.target_pose.header.frame_id = "map";       //living1
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 2.515;
	goal.target_pose.pose.position.y = -2.338;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(0);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	
	tts("开始认识第一个人");
	while(name_num<=2)   //认识三个人
		face_memorize();
	sleep(10);
	
	goal.target_pose.header.frame_id = "map";    //living2
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 3.802;
	goal.target_pose.pose.position.y = -4.141;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.14/2);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	
	goal.target_pose.header.frame_id = "map";    //kitchen2
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 2.019;
	goal.target_pose.pose.position.y = -7.212;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.14/2);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	
	goal.target_pose.header.frame_id = "base_link";    //base_link
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 1;
	goal.target_pose.pose.position.y = 0;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(0);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	
	for(int j=0;j<=2;j++)   //注册三个人的人脸信息
	register_image(FD,PD,FR,filenames[j]);
	register_image(FD,PD,FR,"//home//robot//duiyuan.jpg");   //注册已知队员的人脸信息
	people_recognition();    //进行识别
	
	
	
	//离场
	
	goal.target_pose.header.frame_id = "base_link";    // 旋转
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 0;
	goal.target_pose.pose.position.y = 0;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(3.14);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	goal.target_pose.header.frame_id = "map";     //dinging 0
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 5.629;
	goal.target_pose.pose.position.y = -4.579;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.14/2);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	goal.target_pose.header.frame_id = "map";     //dinging 1
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 7.810;
	goal.target_pose.pose.position.y = -5.734;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.14/2);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	goal.target_pose.header.frame_id = "map";     //dinging 2
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 8.135;
	goal.target_pose.pose.position.y = -8.813;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.14/2);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	goal.target_pose.header.frame_id = "map";     //出口前
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 7.467;
	goal.target_pose.pose.position.y = -10.116;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.14/2);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	move();
	MSPLogout(); //退出登录
	return 0;
}
