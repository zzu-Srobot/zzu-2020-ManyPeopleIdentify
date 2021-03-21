//按照position相对信息找人  
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
#include <map>
#include <cmath>


using namespace cv;
using namespace std;
using namespace cv::dnn;

seeta::FaceDetector2 FD("/home/robot/catkin_ws/include/SeetaFaceDetector2.0.ats");
seeta::FaceRecognizer2 FR("/home/robot/catkin_ws/include/SeetaFaceRecognizer2.0.ats"); 
seeta::PointDetector2 PD("/home/robot/catkin_ws/include/SeetaPointDetector2.0.pts5.ats");
const size_t inWidth = 300;
const size_t inHeight = 300;
const float inScaleFactor = 0.007843f;
const float meanVal = 127.5;
string Names[]={"Alex","Angel","Edward","Homer","Jamie","John","Kevin","Kurt","Tracy","Robin","Eve","Jane","Liza","Melissa","Sophia","person one","person two","person three"};
string Things[]={"sponge","toilet paper","soap","lotion","toothpaste","cloth","green_tea","papaya_milk","pure_milk","water","orange_juice","beer","biscuits","bubble gum","chocolates","tomato chips","barbecue chips","honey chips","coconut cereals","coco balls","egg stars","gram soup","bean sauce","pear","lemon","apple","bowl","plate","tray","barbecue chips","chocolates","green_tea"};
string filenames[4];//存储人脸照片

int counts=0;//识别结果图片名称标号递增
int source_counts = 0; //注册结果图片名称标号递增
int unknown_id = 15;  //未正确识别出名字的暂时称呼标号
int unknown_id2 =0;//未能识别出物品的暂时称呼编号
map<int,string> name_id;//对应id的名字

void people_recognition(Mat& mat);
typedef actionlib::SimpleActionClient<move_base_msgs::MoveBaseAction> MoveBaseClient;//////////////////////////////
const float depthThreshold = 3.7; //设置检测到的最远距离
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
unsigned int ques_find_or(string str_org, initializer_list<string> lst)
{
	unsigned int find_num = 0;
	for (auto i = lst.begin(); i != lst.end(); i++)
	{
		if (str_org.find(*i) != str_org.npos)
		{
			//cout<<*i<<endl;
			return 1;
		}
	}
	return 0;
}
string lower(string raw){
	for(int i=0; i<raw.size();++i)
		if(raw[i] >= 'A' && raw[i] <= 'Z')
			raw[i] = raw[i] + 32;
	return raw;
}
int find_name(string quest_str)
{
	if(ques_find_or(quest_str, {"alex","lex","alex","alice"}) ) return 0;// "Alex";
	else if(ques_find_or(quest_str, {"angel","gel","ein"})) return 1;// "Angel";
	else if(ques_find_or(quest_str, {"word","add","edward","edward"})) return 2;// "Edward";
	else if(ques_find_or(quest_str, {"home","mer","homer"})) return 3;// "Homer";
	else if(ques_find_or(quest_str, {"jame","mi","jamie","jimmy","jimmy"})) return 4;// "Jamie";
	else if(ques_find_or(quest_str, {"john","john"})) return 5;// "John";
	else if(ques_find_or(quest_str, {"vin","kei","kevin"})) return 6;// "Kevin";
	else if(ques_find_or(quest_str, {"te","kurt","colt","cook"})) return 7;// "Kurt";
	else if(ques_find_or(quest_str, {"tree","tricky","tracy","tracy"})) return 8;// "Tracy";
	else if(ques_find_or(quest_str, {"robin","bin"})) return 9;// "Robin";
	else if(ques_find_or(quest_str, {"eve","if","even","eve"})) return 10;// "Eve";
	else if(ques_find_or(quest_str, {"jane","jan","gene"})) return 11;// "Jane";
	else if(ques_find_or(quest_str, {"liza","lee","lisa",})) return 12;// "Liza";
	else if(ques_find_or(quest_str, {"melissa","mei","lissa"})) return 13;// "Melissa";
	else if(ques_find_or(quest_str, {"so","sophia"})) return 14;// "Sophia";
	else return -1;
}
int find_thing(string quest_str)
{
	if(ques_find_or(quest_str, {"spon","sponge","bang","bunch","punch"}))return 0;// sponge;
	else if(ques_find_or(quest_str, {"toilet","paper","per"})) return 1;// toilet paper;
	else if(ques_find_or(quest_str, {"soap","soup","stop"})  && !ques_find_or(quest_str, {"gram","graham","grab"}) ) return 2;// soap;
	else if(ques_find_or(quest_str, {"lotion","loo","loosen","location"})) return 3;// lotion;*****
	else if(ques_find_or(quest_str, {"tooth","paste","truth","taste"})) return 4;// toothpaste;
	else if(ques_find_or(quest_str, {"cloth","close"})) return 5;// cloth;
	
	else if(ques_find_or(quest_str, {"lemon"})) return 24;// lemon;
	
	else if(ques_find_or(quest_str, {"green","tea"})) return 6;// green_tea;
	else if(ques_find_or(quest_str, {"papaya","prepare","papa"})) return 7;// papaya_milk;****
	else if(ques_find_or(quest_str, {"pure","pill","pure milk"})) return 8;// pure_milk
	else if(ques_find_or(quest_str, {"water","wo"})) return 9;// water;
	else if(ques_find_or(quest_str, {"orange","juice","joe"})) return 10;// orange_juice;
	else if(ques_find_or(quest_str, {"beer"})) return 11;// beer;///
	else if(ques_find_or(quest_str, {"biscuit","kiss","cake"})) return 12;// biscuits;
	else if(ques_find_or(quest_str, {"gum","double","bible","bubble"})) return 13;// bubble gum;
	else if(ques_find_or(quest_str, {"late","choco","chocolate","talk"})) return 14;// chocolates;
	else if(ques_find_or(quest_str, {"tomato","mato",})) return 15;// tomato chips;
	else if(ques_find_or(quest_str, {"bob","barbecue","cue"})) return 16;// barbecue chips;*****
	else if(ques_find_or(quest_str, {"honey","ney"})) return 17;// honey chips;
	else if(ques_find_or(quest_str, {"coconut","conut","reals","cereal","serious"})) return 18;// coconut cereals;
	else if(ques_find_or(quest_str, {"coco","ball"})) return 19;// coco balls;
	else if(ques_find_or(quest_str, {"egg","star","bar","egg"}))return 20;// egg stars;
	else if(ques_find_or(quest_str, {"gram","graham","grab"})) return 21;// gram soup;///////   注意和soap分别
	else if(ques_find_or(quest_str, {"bean","sauce","south"})) return 22;// bean sauce;
	else if(ques_find_or(quest_str, {"pear","pair","ear","peer"})) return 23;// pear;****
	
	else if(ques_find_or(quest_str, {"apple","ple"})) return 25;// apple;
	else if(ques_find_or(quest_str, {"bowl","ball"})) return 26;// bowl; //注意和coco balls区分
	else if(ques_find_or(quest_str, {"plate","late","plight","play"})) return 27;// plate;
	else if(ques_find_or(quest_str, {"tray","train","tree","try"})) return 28;// tray;
	else return -1;
}
////////////
void face_memorize(Receiver & receiver)
{
	cv::Mat mat,depth;
	seeta::cv::ImageData image;
	SeetaRect *face;
	SeetaPointF *points;
	int recog_count=0;
	int name_index;
	int thing_index;
	//tts("你好，请靠近我并大声说出你的名字");
	tts("Hello,what's your name");
		while(1)
	{
		
		string get_result=lower(recog());
		
		recog_count++;
		
        if(get_result.empty()) 
        {
			if(recog_count<=5)
			{
			tts("Please say it again");
			continue;
			}
		}
		name_index=find_name(get_result);
		if(name_index!=-1)
		{
			tts("I remember you");
			string str_all="Your name is "+Names[name_index];
			name_id[source_counts] = Names[name_index];
			
			const char* p = str_all.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			cout<<str_all<<endl;
			break;
		}
		else if(recog_count>5)
		{
			name_index = unknown_id++;
			
			tts("Sorry, I can't hear your name clearly");//对不起，我不能听清楚你的名字
			string str_all="I will call you "+Names[name_index];//暂时称呼你为
			name_id[source_counts] = Names[name_index];
			const char* p = str_all.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			cout<<str_all<<endl;
			break;
		}
		else
		{
			tts("Please say it again");//请再说一遍
			continue;
		}
	}
		////////////////////
		tts("What do you want me to help you find");//你希望我帮助你寻找的物品是什么呢
		recog_count=0;
		while(1)
	{
		
		string get_result2=lower(recog());
		recog_count++;
		
        if(get_result2.empty()) 
        {
			if(recog_count<=5)
			{
			tts("Please say it again");//请再说一遍
			continue;
			}
		}
		thing_index=find_thing(get_result2);
		if(thing_index!=-1)
		{
			tts("I remember");
			string str_all2="I will help you find "+Things[thing_index];//你希望我帮助你寻找的物品是
			cout<<str_all2<<endl;
			const char* p = str_all2.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			
			
			break;
		}
		else if(recog_count>5)
		{
			thing_index= unknown_id2++;
			
			tts("Sorry, I can't hear you clearly");
			string str_all2="I think you want to "+Things[thing_index];
			cout<<str_all2<<endl;
			const char* p = str_all2.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			
			
			break;
		}
		else
		{
			tts("Please say it again");
			continue;
		}
		////////////////////
	}
		tts("Now, take photos! Please hold still!");//正前方只有一个人
		
		
		tts("I'll take pictures in five seconds");//我将在五秒后进行拍照
		sleep(3);
	
		while(1)
	{
		receiver.imageViewer(mat, depth, 1);
		image=mat;
		face = FD.Detect(image);

		if(face)
		{
			rectangle(mat,Rect(face->x,face->y,face->width,face->height),Scalar(255,0,0),1);
			filenames[source_counts]="//home//robot//zzu-sr_whoiswho_result//" + to_string(source_counts+100) + ".jpeg";
			putText(mat, Things[thing_index], Point((int)( 100 ),
						(int)( 100 )), FONT_HERSHEY_SIMPLEX, 1, Scalar(180, 105, 255), 3);
			imwrite("//home/robot//zzu-sr_whoiswho_result//" + to_string(source_counts + 100) + ".jpeg",mat);
			source_counts++;
			sleep(1);
			tts("I remember you ");//我记住你的模样了
			break;
		}
		else
		{
			tts("Recognition failed. Please adjust the position and take photos again");//识别失败，请调整位置重新拍照
			continue;
		}
	}
}
void move(ros::Publisher &v_pub)
{
		
		int rate=100;
		int flag=0;
		ros::Rate loop_rate(rate);
		move_base_msgs::MoveBaseGoal goal;
		double goal_angle,angle_duration;
		int ticks;
		geometry_msgs::Twist vel;
		
		float linear_speed,goal_distance,linear_duration;
		goal_distance = 1.5;
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
		for(int i=0; i<10; ++i){
			v_pub.publish(vel);
			loop_rate.sleep();
		}
}

void stop(ros::Publisher &v_pub){
	geometry_msgs::Twist vel;
	vel.linear.x = 0;
	vel.angular.z = 0;	
	ros::Rate loop_rate(10);
	for(int i=0; i<10; ++i){
		v_pub.publish(vel);
		loop_rate.sleep();
	}
}

void rotation_finding(bool flag , ros::Publisher& v_pub){
	
	geometry_msgs::Twist vel;
	
	String modelConfiguration = "/home/robot/catkin_ws/include/MobileNetSSD_deploy.prototxt";
	String modelBinary = "/home/robot/catkin_ws/include/MobileNetSSD_deploy.caffemodel";
	Net net = readNetFromCaffe(modelConfiguration, modelBinary);
	
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
	
	unordered_map <int , int>  umap;
	ros::Rate loop_rate1(10);
	
	while(true)
	{
		
		receiver.imageViewer(color, depth, 1);
		Mat inputBlob = blobFromImage(color, inScaleFactor, Size(inWidth, inHeight), Scalar(meanVal, meanVal, meanVal), false, false);
		net.setInput(inputBlob); 		
		Mat detection = net.forward("detection_out"); 								 
		Mat detectionMat(detection.size[2], detection.size[3], CV_32F, detection.ptr<float>());
		float confidenceThreshold = 0.36;
		
		
		
			
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
				if(( objectClass==15 )&&( confidence > 0.5 ))//objectClass==15代表是人类的分类
				{
					rectangle(color, Point(tl_x, tl_y), Point(br_x, br_y), Scalar(0, 255, 0), 3);

					const uint16_t *itD = depth.ptr<uint16_t>((int)( 0.5 * (tl_y + br_y) ));
//优化距离********  **  **	*	***********************	
	    			
	   				for(int i= tl_x; i<= br_x; ++i){
						int xp = (*(itD+i) / 1000.0f)*10;
						if( xp>1&&xp<= depthThreshold*10){ //只筛选出在0.1米到4.5米之间距离的像素点
							umap[xp] ++;
							//cout << xp <<" ";
						}
					}
					int mmax = -1;
					float distance = -1;
					for(auto& depth:umap){
						if(depth.second > mmax){
							mmax = depth.second;
							distance = depth.first;
						}
					}
					
					
					float final_distance = distance/10;//最终确定的距离
	    			float depthValue = final_distance;
//********************************************************************************	  
				if(depthValue>0.1&&depthValue<=depthThreshold){
	    			muldepth.push_back(depthValue);
	    			
	    			
	    			putText(color, to_string(depthValue), Point((int)( 0.5 * (tl_x + br_x) ), (int)( 0.5 * (tl_y + br_y) )), FONT_HERSHEY_SIMPLEX, 3, Scalar(180, 105, 255), 3);
					
					float position=(tl_x+br_x)/9.60/2;

					mulposition.push_back(position);	
				}	
				}
				
				umap.clear();
			}	
			
		  }	
		  int j=0;//对应mindepth的position下标
		  int i=0;
		  float mindepth = 10000;//哨兵
				for(vector<float>::iterator it=muldepth.begin();it<muldepth.end();++i,++it)
					{
						if(*it<mindepth)
						{
							j=i;
							mindepth = *it;
						}
					}
					float minposition = 0;
					if(mindepth<9999)
					{
						minposition = mulposition[j];
						
					}
			
			
		if( !(mindepth>0.01&&mindepth<=depthThreshold) ){ //距离检测过小，或者太远，都视为不合法
								
								
								
										vel.linear.x= 0;
										vel.linear.y = 0;
										vel.linear.z = 0;
										vel.angular.x= 0;
										vel.angular.y = 0;
										vel.angular.z = 0.25;
										
										v_pub.publish(vel);
										imshow("color",color);
										muldepth.clear();//制空
										mulposition.clear();
									    continue;
		}
		
		
		
		if(mindepth<9999)//检测到人
		{		
//------------------------------------------------------------------
		if(minposition<40&&minposition>=0)
					{
						
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = 0.5;
					}
					else if(minposition>=60&&minposition<99)
					{
						
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = -0.5;
					}	
					else
					{				
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = 0;	
					}
//---------------------------------------------------------------					
					if(mindepth>1.0&&mindepth<depthThreshold+0.01)	
					{
						vel.linear.x= 0.25;
						vel.linear.y = 0;
						vel.linear.z = 0;
					}
					else if(mindepth>0&&mindepth<0.6)	
					{
						vel.linear.x= -0.2;
						vel.linear.y = 0;
						vel.linear.z = 0;
					}
					else
					{
						vel.linear.x= 0;
						vel.linear.y = 0;
						vel.linear.z = 0;
						
					}
//---------------------------------------------------------------------					
					
					v_pub.publish(vel);
					if(fabs(vel.linear.x) <0.01&&fabs(vel.linear.y) < 0.01 && fabs(vel.linear.z) < 0.01&&fabs(vel.angular.x)< 0.01&&fabs(vel.angular.y) < 0.01&&fabs(vel.angular.z)< 0.01&&mindepth>=0.6&&mindepth<=1.0001)//找到人
							{
								for(int i=0; i<3; ++i)
									v_pub.publish(vel);
									
									
								tts("I find!");
									
								
								if(flag){//识别
									
										tts("Please hold still.");
										sleep(3);
										receiver.imageViewer(color, depth, 1);
										people_recognition(color);
										
									
								}
								else{
									//拍照，注册...............
									face_memorize(receiver);
								}
								
								//tts("Next!");
								ros::Rate loop_rat(10);
								int count=0;
								while(count<20)
									{
										vel.linear.x= 0;
										vel.linear.y = 0;
										vel.linear.z = 0;
										vel.angular.x= 0;
										vel.angular.y = 0;
										vel.angular.z = 0.5;
										
										
										v_pub.publish(vel);
										
										loop_rat.sleep();
										count++;
									}
							break; //退出，找下一个
							}
		//-------------------------------------------------------------------
		
	}
		else//找不到人，旋转
			{
				vel.linear.x= 0;
				vel.linear.y = 0;
				vel.linear.z = 0;
				vel.angular.x= 0;
				vel.angular.y = 0;
				
			   vel.angular.z = 0.25;
				
				v_pub.publish(vel);
			}
		muldepth.clear();//制空
		mulposition.clear();
		
		imshow("color",color);
	}
}
struct face_similar{
	int index;
	float similar;
	SeetaRect *face;
	face_similar(int a, float b, SeetaRect *f): index(a),similar(b),face(f){}
	bool operator < (const face_similar& other) const{
		return similar > other.similar;
	}
};
unsigned int rec_num = 0;
void people_recognition(Mat& mat)
{	
	string response;
	seeta::cv::ImageData image;
	SeetaRect *face;
	SeetaPointF *points;
	float similar=0;
	int num,index;
	image = mat;
	vector<face_similar> all_faces;
		face= FD.Detect(image,&num);
		
		for (int i = 1; i <= num; ++i,face++)
			{
				rectangle(mat,Rect(face->x,face->y,face->width,face->height),Scalar(255,0,0),3);
				points = PD.Detect(image, *face);
				index = FR.Recognize(image, points, &similar);
				
				
				
					
				all_faces.push_back(face_similar(index,similar,face));
			}
		if(all_faces.size()==0)
			return;
		
		sort(all_faces.begin(),all_faces.end());
		index = all_faces[0].index;
		face =  all_faces[0].face;
		response ="Hello! Your name is ";
		response += name_id[index];
		tts(response.data());
		cout<<response<<endl;
		putText(mat, name_id[index], Point((int)( face->x + 0.5 * face->width ),
						(int)( face->y + 0.5 * face->height )), FONT_HERSHEY_SIMPLEX, 1, Scalar(180, 105, 255), 3);
		sleep(1);
		imwrite("//home//robot//zzu-sr_whoiswho_result//" + to_string(counts++) + ".jpeg",mat);	
		++rec_num;
}
void introduce_robot(){
	tts("Hello, I'm Emily. I am from the service robot laboratory of Zhengzhou University.");
	tts("I'm pleased to meet you.");
}
int main(int argc, char *argv[])
{
	const char* login_params = "appid = 5c7a2954, work_dir = .";//"appid = 5caf019f, work_dir = .";//5aa8ea00  5c7a2954

	MSPLogin(NULL, NULL, login_params);
	
	ros::init(argc, argv, "multi_recognation");
	ros::NodeHandle nh;
	ros::Publisher v_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 200, true);
	tts("Begin!");
		
	move(v_pub);	
		
	move_base_msgs::MoveBaseGoal goal;
	MoveBaseClient ac("move_base", true); 
	ac.waitForServer(ros::Duration(5.0));
	goal.target_pose.header.frame_id = "map";       //进场到指定位置进行人脸识别
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 2.281;
	goal.target_pose.pose.position.y = -2.177;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-1.679);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	
	sleep(1.5);


	
	introduce_robot(); //自我介绍
	
	for(int i=0; i<3; ++i){//找人注册脸
		rotation_finding(false,v_pub);
	}
	for(int i=0; i<3; ++i){	
		register_image(FD,PD,FR,filenames[i]);
	}	
	cout<<"开始识别脸"<<endl;
	stop(v_pub);
	tts("Start recognition!");
	sleep(10);   //停留10s再开始人脸识别，或者导航离开再回来
	while(rec_num<3)//识别脸
		rotation_finding(true,v_pub);
		
	goal.target_pose.header.frame_id = "map";       
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 2.269;
	goal.target_pose.pose.position.y = -4.121;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-1.431);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);
	
	goal.target_pose.header.frame_id = "map";      
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 2.263;
	goal.target_pose.pose.position.y = -5.301;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-1.519);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);	

	goal.target_pose.header.frame_id = "map";       //这个点可以考虑直接标在门外，防止撞墙
	goal.target_pose.header.stamp = ros::Time::now();
	goal.target_pose.pose.position.x = 0.458;
	goal.target_pose.pose.position.y = -7.959;
	goal.target_pose.pose.orientation = tf::createQuaternionMsgFromYaw(-3.115);
	ROS_INFO("Sending goal");
	ac.sendGoal(goal);
	ac.waitForResult();
	if(ac.getState() == actionlib::SimpleClientGoalState::SUCCEEDED) ROS_INFO("succeeded");
	else ROS_INFO("failed");
	sleep(1.5);	


	move(v_pub);	
	MSPLogout(); 
	return 0;
}
