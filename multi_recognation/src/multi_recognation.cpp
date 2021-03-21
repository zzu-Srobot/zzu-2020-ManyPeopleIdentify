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
string Names[]={"James","Alex","Ryan","John","Eric","Adam","Carter","Jack","David","Tyler","Lily","Mary","Anna","Zoe","Sara","Sofia","Faith","Julia","Paige","Jessica","person one","person two","person three"};

string filenames[4];//存储人脸照片

int counts=0;//识别结果图片名称标号递增
int source_counts = 0; //注册结果图片名称标号递增
int unknown_id = 20;  //未正确识别出名字的暂时称呼标号

map<int,string> name_id;//对应id的名字
void people_recognition(Mat& mat);

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
	else if(ques_find_or(quest_str, {"nna"})) return 12;// "Anna";
	else if(ques_find_or(quest_str, {"zo","ee"})) return 13;// "Zoe";
	else if(ques_find_or(quest_str, {"ra","eve"})) return 14;// "Sara";
	else if(ques_find_or(quest_str, {"fia","phie"})) return 15;// "Sofia";
	else if(ques_find_or(quest_str, {"aith","ace"}) )return 16;// "Faith";
	else if(ques_find_or(quest_str, {"lia"})) return 17;// "Julia";
	else if(ques_find_or(quest_str, {"aige","age"})) return 18;// "Paige";
	else if(ques_find_or(quest_str, {"essica"}) )return 19;// "Jessica";
	else return -1;
}

void face_memorize(Receiver & receiver)
{
	cv::Mat mat,depth;
	seeta::cv::ImageData image;
	SeetaRect *face;
	SeetaPointF *points;
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
			name_id[source_counts] = Names[name_index];
			const char* p = str_all.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			break;
		}
		else if(recog_count>5)
		{
			name_index=unknown_id++;
			
			tts("对不起，我听不清你的名字");
			string str_all="暂时称呼你为"+Names[name_index];
			name_id[source_counts] = Names[name_index];
			const char* p = str_all.data();//string型转*char型，tts()入口参数为*char型
			tts(p);//语音复述一遍
			break;
		}
		else
		{
			tts("请再说一遍");
			continue;
		}
	}
		tts("现在开始拍照");//正前方只有一个人
		
		tts("请其他人不要在我面前，以免造成干扰");
		tts("我将在五秒后进行拍照");
		while(1)
	{
		receiver.imageViewer(mat, depth, 1);
		image=mat;
		face = FD.Detect(image);

		if(face)
		{
			rectangle(mat,Rect(face->x,face->y,face->width,face->height),Scalar(255,0,0),1);
			filenames[source_counts]="//home//robot//zzu-sr_whoiswho_result//" + to_string(source_counts+100) + ".jpeg";
			imwrite("//home/robot//zzu-sr_whoiswho_result//" + to_string(source_counts + 100) + ".jpeg",mat);
			source_counts++;
			sleep(1);
			tts("我记住你的模样了");
			break;
		}
		else
		{
			tts("识别失败，请调整位置重新拍照");
			continue;
		}
	}
}


void rotation_finding(bool flag , ros::Publisher& v_pub){
	
	geometry_msgs::Twist vel;
	
	String modelConfiguration = "/home/robot/catkin_ws/src/IJCAI/daily_service/find_human/src/MobileNetSSD_deploy.prototxt";
	String modelBinary = "/home/robot/catkin_ws/src/IJCAI/daily_service/find_human/src/MobileNetSSD_deploy.caffemodel";
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
	
	while(waitKey(30)!=27)
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
//优化距离*****  **  **	*	***********************	
	    			cout<<"-------------------------------------"<<endl;
	   				for(int i= tl_x; i<= br_x; ++i){
						int xp = (*(itD+i) / 1000.0f)*100;
						if( xp>10&&xp< 450){ //只筛选出在0.1米到4.5米之间距离的像素点
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
					
					
					float final_distance = distance/100;//最终确定的距离
	    			float depthValue = final_distance;
//********************************************************************************	    			
	    			muldepth.push_back(depthValue);
	    			cout<<"depthvalue:"<<depthValue<<endl;
	    			
	    			putText(color, to_string(depthValue), Point((int)( 0.5 * (tl_x + br_x) ), (int)( 0.5 * (tl_y + br_y) )), FONT_HERSHEY_SIMPLEX, 3, Scalar(180, 105, 255), 3);
					
					float position=(tl_x+br_x)/9.60/2;
					cout<<"position:"<<position<<endl;	
					mulposition.push_back(position);		
				}
				else//找不到人，旋转
				{
						vel.linear.x= 0;
						vel.linear.y = 0;
						vel.linear.z = 0;
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = -0.1;
						cout<<"内转"<<endl;
						v_pub.publish(vel);
						imshow("color",color);
						continue;
				}
				umap.clear();
			}	
			else//找不到人，旋转
				{
						vel.linear.x= 0;
						vel.linear.y = 0;
						vel.linear.z = 0;
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = -0.1;
						cout<<"外转"<<endl;
						v_pub.publish(vel);
						imshow("color",color);
						continue;
				}
		  }	
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
					float mindepth = 0;
					if(minposition<1000)
					{
						mindepth = muldepth[j];
						cout<<"mindepth:"<<mindepth<<endl;
					}
			cout<<"minposition:"<<minposition<<endl;
			
		if( !(mindepth>0.01&&mindepth<4.5) ){ //距离检测过小，或者太远，都视为不合法
								
								
								
										vel.linear.x= 0;
										vel.linear.y = 0;
										vel.linear.z = 0;
										vel.angular.x= 0;
										vel.angular.y = 0;
										vel.angular.z = -0.1;
										cout<<"距离检测过小，或者太远，都视为不合法"<<mindepth<<endl;
										v_pub.publish(vel);
										imshow("color",color);
									    continue;
		}
		
		
		
		if(minposition<1000)//检测到人
		{		
//------------------------------------------------------------------
		if(minposition<40&&minposition>1)
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
					{	cout<<"angualr:0"<<endl;				
						vel.angular.x= 0;
						vel.angular.y = 0;
						vel.angular.z = 0;	
					}
//---------------------------------------------------------------					
					if(mindepth>1.0&&mindepth<4.5)	
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
					
					geometry_msgs::Twist vel_find;
					vel_find=vel;
					v_pub.publish(vel);
					if(vel_find.linear.x <0.01&&vel_find.linear.y < 0.01 && vel_find.linear.z < 0.01&&vel_find.angular.x< 0.01&&vel_find.angular.y < 0.01&&vel_find.angular.z< 0.01&&mindepth>0.6&&mindepth<1.0)//找到人
							{
								tts("I find!");
									
								
								if(flag){//识别
									tts("Please hold still.");
									sleep(2);
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
								while(count<25)
									{
										vel.linear.x= 0;
										vel.linear.y = 0;
										vel.linear.z = 0;
										vel.angular.x= 0;
										vel.angular.y = 0;
										vel.angular.z = -0.5;
										
										cout<<"flag3"<<endl;
										v_pub.publish(vel);
										cout<<"flag4"<<endl;
										loop_rat.sleep();
										count++;
									}
							break; //退出，找下一个
							}
		//-------------------------------------------------------------------
		
	}
		
		muldepth.clear();//制空
		mulposition.clear();
		loop_rate1.sleep();
		imshow("color",color);
	}
}

void people_recognition(Mat& mat)
{	
	
	
	string response;
	seeta::cv::ImageData image;
	SeetaRect *face;
	SeetaPointF *points;
	float similar=0;
	int num,index;
	image = mat;
	face= FD.Detect(image,&num);
	if(!num)
		return ;
	
		response ="Hello!Your name is ";
		rectangle(mat,Rect(face->x,face->y,face->width,face->height),Scalar(255,0,0),3);
		points = PD.Detect(image, *face);
		index = FR.Recognize(image, points, &similar);//有点问题。。。。。
		cout<<"index:"<<index<<endl;
		cout<<"similar:"<<similar<<endl;
		
		putText(mat, name_id[index], Point((int)( face->x + 0.5 * face->width ),
			(int)( face->y + 0.5 * face->height )), FONT_HERSHEY_SIMPLEX, 1, Scalar(180, 105, 255), 3);
		response += name_id[index];
		tts(response.data());
		
		imwrite("//home//robot//zzu-sr_whoiswho_result//" + to_string(counts++) + ".jpeg",mat);	
}

int main(int argc, char *argv[])
{
	
	const char* login_params = "appid = 5c7a2954, work_dir = .";//"appid = 5caf019f, work_dir = .";//5aa8ea00  5c7a2954

	MSPLogin(NULL, NULL, login_params);
	
	ros::init(argc, argv, "multi_recognation");
	ros::NodeHandle nh;
	ros::Publisher v_pub = nh.advertise<geometry_msgs::Twist>("cmd_vel", 200, true);
	tts("begin");
		
		
		
	for(int i=0; i<2; ++i){//找人注册脸
		rotation_finding(false,v_pub);
	}
	for(int i=0; i<2; ++i){	
		register_image(FD,PD,FR,filenames[i]);
	}	
	cout<<"开始识别脸"<<endl;
	tts("Start recognition!");
	
	for(int i=0; i<2; ++i)//识别脸
		rotation_finding(true,v_pub);
	MSPLogout(); 
	return 0;
}
