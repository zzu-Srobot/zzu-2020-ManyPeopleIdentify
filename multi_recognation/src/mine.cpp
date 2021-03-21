#include "std_msgs/String.h"
#include "ros/ros.h"
#include <stdlib.h>
#include <string.h>
#include <c++/5.4.0/iostream>
#include <c++/5.4.0/sstream>
#include <c++/5.4.0/string>
#include <c++/5.4.0/stdexcept>
#include <vector>
#include <glob.h>
#include <opencv2/opencv.hpp>
#include <unistd.h>
#include <stdio.h>
#include <opencv2/dnn.hpp>
#include <geometry_msgs/Twist.h>
#include "std_msgs/String.h"
#include "std_msgs/Int8.h"
#include "recogfile.h"
#include "tts.h"

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
#include <seeta/FaceDetector2.h>
#include <seeta/FaceRecognizer.h>
#include <seeta/PointDetector2.h>
#include <seeta/Struct_cv.h>
#include <actionlib/client/simple_action_client.h>
#include <move_base_msgs/MoveBaseAction.h>
#include <tf/transform_datatypes.h>
#include <tf/transform_listener.h>
#include <initializer_list>


#define camID 0
#define FACENUM 5
#define _USE_MATH_DEFINES



using namespace cv; src ljcal dialy find.cpp 10_2fhh.cpp
using namespace std;
using namespace cv::dnn;

const size_t inWidth = 300;
const size_t inHeight = 300;
const float inScaleFactor = 0.007843f;
const float meanVal = 127.5;

int name_num=0;
int counts=0;

int follow_allow = 1;//zyh 无需进入回调  原值为0


int main(int argc, char *argv[])
{
	const char* login_params = "appid = 5c7a2954, work_dir = .";//"appid = 5caf019f, work_dir = .";//5aa8ea00  5c7a2954

	MSPLogin(NULL, NULL, login_params);

	
	ros::init(argc, argv, "find_human");
	ros::NodeHandle nh;
	tts("begin");
	int rate =30;
	
	ros::Rate loop_rate(rate);
	geometry_msgs::Twist vel;
	ros::Publisher v_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 200, true);
	ros::Subscriber f_sub = nh.subscribe("follow_allowance", 10, chatterCallback);
	
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
	Mat color, depth;
	vector<float>  muldepth;//一张图片，存多个人的距离信息
	vector<float>  mulposition;
	
	while(waitKey(27)!=30)
	{
		receiver.imageViewer(color, depth, 1);
		Mat inputBlob = blobFromImage(color, inScaleFactor, Size(inWidth, inHeight), Scalar(meanVal, meanVal, meanVal), false, false);
		net.setInput(inputBlob); 		
		Mat detection = net.forward("detection_out"); 								 
		Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
		float confidenceThreshold = 0.36;
		
		if(follow_allow == 1)
		{
			
		for (int i = 0; i < detectionMat.rows; i++)
		{
			float confidence = detectionMat.at<float>(i, 2);
			if (confidence > confidenceThreshold)	
			{
				
				size_t objectClass = (size_t)(detectionMat.at<float>(i, 1));
				int tl_x = static_cast<int>(detectionMat.at<float>(i, 3) * color.cols);
				int tl_y = static_cast<int>(detectionMat.at<float>(i, 4) * color.rows);
				int br_x = static_cast<int>(detectionMat.at<float>(i, 5) * color.cols);
				int br_y = static_cast<int>(detectionMat.at<float>(i, 6) * color.rows);
				if(( objectClass==15 )&&( confidence > 0.5 )&&(follow_allow == 1))//objectClass==15代表是人类的分类
				{
					rectangle(color, Point(tl_x, tl_y), Point(br_x, br_y), Scalar(0, 255, 0), 3);

					const uint16_t *itD = depth.ptr<uint16_t>((int)( 0.5 * (tl_y + br_y) ));
	   				itD += (int)( 0.5 * (tl_x + br_x) );
	    			//register const float depthValue = *itD / 1000.0f;//depthValue单位为米 register??
	    			register const float depthValue = *itD/ 1000.0f;
	    			muldepth.push_back(depthValue);
	    			cout<<"depthvalue:"<<depthValue<<endl;
	    			
	    			putText(color, to_string(depthValue), Point((int)( 0.5 * (tl_x + br_x) ), (int)( 0.5 * (tl_y + br_y) )), FONT_HERSHEY_SIMPLEX, 3, Scalar(180, 105, 255), 3);
					
					float position=(tl_x+br_x)/9.60/2;
					cout<<"position:"<<position<<endl;	
					mulposition.push_back(position);		
				}
			}	
		  }	
		  
		  //获得合适的depthvalue
		  int j=0;//对应position的depthValue下标
		  int i=0;
		  float minposition = 1000;//哨兵
				for(vector<float>::iterator it=mulposition.begin();it<mulposition.end();++i,++it)
					{
						if(*it<minposition)
						{
							j=i;
							minposition = *it;
						}
					}
					float mindepth;
					if(minposition<1000)
					{
						mindepth = muldepth[j];
						cout<<"mindepth:"<<mindepth<<endl;
					}
			cout<<"minposition:"<<minposition<<endl;
			
			
		if(minposition<1000)//检测到人
		{		
		//------------------------------------------------------------------
					if(minposition<30&&minposition>1)
					{
						cout<<minposition<<endl;
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = 0.5;
					}
					else if(minposition>=70&&minposition<99)
					{
						cout<<minposition<<endl;
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = -0.5;
					}	
					else
					{	cout<<"angualr:0"<<endl;				
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = 0;	
					}
					
					if(mindepth>0.6&&mindepth<5.0)	
					{
						vel.linear.x= 0.25;
						vel.linear.y = 0;
						vel.linear.z = 0;
					}
					else if(mindepth>0&&mindepth<0.3)	
					{
						vel.linear.x= -0.2;
						vel.linear.y = 0;
						vel.linear.z = 0;
					}
					else if(mindepth>5.0)
					{
						vel.linear.x= 0;
						vel.linear.y = 0;
						vel.linear.z = 0;
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = 0;
						cout << "debug"<< endl;
						//tts("Sorry, I can't find you.");
					}
					else
					{
						vel.linear.x= 0;
						vel.linear.y = 0;
						vel.linear.z = 0;
						
					}
					cout<<"published something"<<endl;
					geometry_msgs::Twist vel_find;
					vel_find=vel;
					v_pub.publish(vel);
					if(vel_find.linear.x <0.01&&vel_find.linear.y < 0.01 && vel_find.linear.z < 0.01&&vel_find.angular.x< 0.01&&vel_find.angular.y < 0.01&&vel_find.angular.z< 0.01&&mindepth>0.3&&mindepth<0.6)//找到人
							{
								//tts("I find!");
								sleep(2);//人脸识别
								//tts("Next!");
								
								ros::Rate loop_rat(10);
								int count=0;
								
							
							}
		//-------------------------------------------------------------------
		
		}
		else{
			vel.linear.x= 0;
			vel.linear.y = 0;
			vel.linear.z = 0;
			vel.angular.x= 0;
			vel.angular.y = 0;
			vel.angular.z = 0.5;
			cout<<"zhuan xiang ing"<<endl;
			v_pub.publish(vel);
		}
		
		//ros::spinOnce();????
		//loop_rate.sleep();
		
		imshow("color",color);
		
		}
		muldepth.clear();//制空
		mulposition.clear();
	}
	
	
}
