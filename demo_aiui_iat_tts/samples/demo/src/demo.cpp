
/*
* Description: iFlytek: (iat + aiui + tts) demo  /    ros --> FIFO --> iFlytek
* Author     : Kun Gan
* Date       : Sep. 2019
*/


#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <iostream>
#include <iomanip>
#include "AIUITest.h"
#include "playWav.h"
#include "demo.h"
// #include "fifo.h"

#include <pthread.h>

extern "C"{

	#include "iat_online.h"
	#include "tts_offline.h"
	#include "speech_recognizer.h"
}

using namespace std;
using namespace aiui;

//改版
extern OrderProcessManager OrderProcessManager;

/* extracted from resultChar */
extern string ExtractResult;   
extern bool AIUI_Done;
// extern bool OrderFinish;

/* type of answer by AIUI*/
extern ANSWER_TYPE AnswerType;

/* audio file path to be played, depends on the setting of SpeechSynthesis() */
const char* wavpath = "demo.wav";  
const char* Greeting              = "您好!请点单！";
// const char* Greeting              = "您好，欢迎来到智能吧台，我是大眼睛服务机器人，我将根据您的特征信息推荐一些适合您的饮品，待会您可在平板上查看推荐选项，决定后直接告诉我即可！";
const char* gender[2]             = {"先生",
									 "女士"};


const char* Response_2_noAnswer   = "不好意思，？"; //根据槽值追问吧？已修正！
const char* Response_2_noSound[]  = {"可以说大声一些吗？我好像没有听见你说话。",
									 "朋友，你有在说话吗？",			
									 "一点声音都听不见呀",	
									 "他们太吵了，我都听不清你说的话了"};

string txt1 = "准备点餐！";
string txt2 = "你好呀！";
string txt3 = "今天天气怎样";

int pipe_fd_wr = -1;
int pipe_fd_rd = -1;


/*Msg from ROS*/
MsgFromROS msgFromROS;
/*Msg to ROS*/
MsgToROS msgToROS;

/*AIUI Agent*/
AIUITester  KEVIN;


/****** msgFromROS Json format **********
*	{
*	  "command":"idle",
*	  "name":"甘坤",
*	  "gender":"male",
*	  "age":"23",
*	}
*****************************************/
void MsgFromROS::extract_Json(const char* key, char* json, char* dest){

	char* start = NULL;
	char* end   = NULL;

	/*extract dest from Json*/
	if(strstr(json, key)!=NULL){
		start = strstr(json, key);
		start += strlen(key);  

		for(end = start; *end != '\"' && *end!='\0'; ++end);
    	memcpy(dest, start, end-start);
    	dest[end-start] = 0;
	}
}


/******* msgToROS Json format *********
*	{
*	  "name":"甘坤",
*	  "emotion":"puzzled",
*     "greetingDone":"false",
*	  "speechRecoResult":"null",
*	  "OrderInfo":
*	   {
*	 	 "DrinkName":"null",
*	 	 "CupNum":"null",
*	 	 "CupType":"null",
*	 	 "Temp":"null",
*	 	 "OrderFinish":"0"
*	   } 
*	}
***************************************/
void MsgToROS::generate_Json(string name, string emotion, bool greetingDone,string speechRecoResult, string DrinkName, string CupNum, string CupType, string Temp, bool OrderFinish, char* destination){

	string json;

	json = "{\"name\":\"" + name + "\",\"emotion\":\"" + emotion + "\",\"greetingDone\":\"";

	greetingDone == true ? json += "true" : json += "false";

	json += "\",\"speechRecoResult\":\"" + speechRecoResult + "\",\"OrderInfo\":{\"DrinkName\":\"" + DrinkName + "\",\"CupNum\":\"" + CupNum + "\",\"CupType\":\"" + CupType + "\",\"Temp\":\"" + Temp + "\",\"OrderFinish\":\""; 

	OrderFinish == true ? json += "true\"}}" : json += "false\"}}";	   

	strcpy(destination,json.c_str());	   
}





/*order thread*/
void* _order(void* arg){

	char textStitching[512];	
	int conversationCnt = 0;

	cout << "createAgent" << endl;
	KEVIN.createAgent();	

	cout << "wakeup" << endl;
	KEVIN.wakeup();	

	while(1){
		
		while(0==strcmp(msgFromROS.cmd,"working")){

			/*First to serve someone*/
			if(conversationCnt==0){

				/*Automatically start the skill*/
				KEVIN.writeText("开始点餐");
				cout << "test_point1" << endl;
				while(AIUI_Done==false);

				cout << "test_point2" << endl;

				cout << "[ 'order' ] command is currently equal to " << "'"<< msgFromROS.cmd  << "'" << endl;
				
				/*greeting first*/
				/*empty the textStitching*/
				strcpy(textStitching,"");
				/*add the name to textStitching*/
				if(0!=strcmp(msgFromROS.name,""))
					strcat(textStitching,msgFromROS.name);

				/*add the gender to textStitching*/
				if(0==strcmp(msgFromROS.gender,"male"))
					strcat(textStitching,gender[0]);

				else if(0==strcmp(msgFromROS.gender,"female"))
					strcat(textStitching,gender[1]);

				/*add the common response text*/
				strcat(textStitching,Greeting);

				/*first to serve someone, express 'honored' emotion*/
				msgToROS.greetingDone = false; //default
				msgToROS.generate_Json(msgFromROS.name, "honored", msgToROS.greetingDone, msgToROS.speechRecoResult, msgToROS.DrinkName, msgToROS.CupNum, msgToROS.CupType, msgToROS.Temp, msgToROS.OrderFinish, msgToROS.Json);
				int res = write(pipe_fd_wr, msgToROS.Json, strlen(msgToROS.Json));
				if(res==-1)  cout << "[ 'order' ] write err! " << endl;

				/*synthesis greeting audio & play*/
				SpeechSynthesis(textStitching);
				playSound(wavpath);

				/*after greeting, set the flag greetingDone=true, and notify ROS*/
				msgToROS.greetingDone = true;
				msgToROS.generate_Json(msgFromROS.name, "honored", msgToROS.greetingDone, msgToROS.speechRecoResult, msgToROS.DrinkName, msgToROS.CupNum, msgToROS.CupType, msgToROS.Temp, msgToROS.OrderFinish, msgToROS.Json);
				res = write(pipe_fd_wr, msgToROS.Json, strlen(msgToROS.Json));
				if(res==-1)  cout << "[ 'order' ] write err! " << endl;
			}

			/*!!!!check the cmd!!!!*/
			if(0!=strcmp(msgFromROS.cmd,"working")) {
				/*specific response here*/
				break;
			}

			/*recording process*/
			SpeechRecognition();

			/*after recording, save the result and write to ROS*/
			msgToROS.speechRecoResult = g_result;
			msgToROS.generate_Json(msgFromROS.name, "", msgToROS.greetingDone, msgToROS.speechRecoResult, msgToROS.DrinkName, msgToROS.CupNum, msgToROS.CupType, msgToROS.Temp, msgToROS.OrderFinish, msgToROS.Json);
			int res = write(pipe_fd_wr, msgToROS.Json, strlen(msgToROS.Json));
			if(res==-1)  cout << "[ 'order' ] write err! " << endl;
			msgToROS.speechRecoResult = "";

			/*!!!!check the cmd!!!!*/
			if(0!=strcmp(msgFromROS.cmd,"working")) {
				/*specific response here*/
				break;
			}

			/*write to cloud*/
			KEVIN.writeText(g_result);

			/*wait until AIUI result available*/
			while(AIUI_Done==false);  
			
			/*AIUI_Done = true*/
			if( AnswerType==Custom || AnswerType==Turing ){	

				SpeechSynthesis((char*)ExtractResult.data());
				playSound(wavpath);
			}
			else if( AnswerType==NoAnswer ){
				
				/*fixed response, save to local*/
				SpeechSynthesis((char*)ExtractResult.data()); 
				playSound(wavpath);
			}
			else if( AnswerType==NoSound ){
				
				SpeechSynthesis(Response_2_noSound[1]);;
				playSound(wavpath);	

				/*fixed response, save to local*/
				// if(OrderProcessManager.n_NoSound>3){
				// 	SpeechSynthesis(Response_2_noSound[1]);;
				// 	playSound(wavpath);
				// 	OrderProcessManager.n_NoSound = 0;
				// }
			}

			conversationCnt++;	
		}

		conversationCnt = 0;

		// cout << "[ 'order' ] command is currently equal to " << "'"<< msgFromROS.cmd  << "'" << endl;	

		usleep(1000*10);//10ms 

	}
}

/*get Json from ROS thread*/
void* _extractJsonFromROS(void *arg){


	// const char *fifo_name = "/home/kevin/myfifo/ros_2_iFlytek";
	const char *fifo_name = "../../../myfifo/ros_2_iFlytek";
	int open_mode = O_RDONLY;
	char buffer[PIPE_BUF+1];
	int bytes_read = 0; 

	//Empty the buffer
	memset(buffer, '\0', sizeof(buffer));
 
	//RD_ONLY & BLOCKING
	pipe_fd_rd = open(fifo_name, open_mode);
	if(pipe_fd_rd == -1)   
		cout << " open ros_2_iFlytek err !" << endl;
	else 
		cout << "open ros_2_iFlytek successfully!" << endl;



	while(1){
 
 		//read operation blocked, wait until ros:write_end.
		bytes_read = read(pipe_fd_rd, buffer, PIPE_BUF);

		if(bytes_read==-1){

			cout << "read err" << endl;
			close(pipe_fd_rd);

			pipe_fd_rd = open(fifo_name, open_mode);
			if(pipe_fd_rd == -1)   cout << " open err " << endl;

		}
		else if(bytes_read>0){

			buffer[bytes_read] = 0;
			strcpy(msgFromROS.Json,buffer); 
		}
		else
			;

		usleep(1000*10); 

		
		msgFromROS.extract_Json(msgFromROS.keyCmd, msgFromROS.Json,msgFromROS.cmd);
	
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "command: " << msgFromROS.cmd << endl;
	
		msgFromROS.extract_Json(msgFromROS.keyName, msgFromROS.Json,msgFromROS.name);
		
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "name: " << msgFromROS.name << endl;
		
		msgFromROS.extract_Json(msgFromROS.keyGender, msgFromROS.Json,msgFromROS.gender);
		
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "gender: " << msgFromROS.gender << endl;

		msgFromROS.extract_Json(msgFromROS.keyAge, msgFromROS.Json,msgFromROS.age);
		
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "age: " << msgFromROS.age << endl;
		
	}
}


#define toROS


int main()
{

	string choice;


	// cout << "input choice :" << endl;


#ifdef toROS
	/****************** Create FIFO: WR_ONLY & BLOCKING *****************/
	// const char *fifo_name = "/home/kevin/myfifo/iFlytek_2_ros";
	const char *fifo_name = "../../../myfifo/iFlytek_2_ros";
	int res = 0;
	const int open_mode = O_WRONLY;   

	if(access(fifo_name, F_OK) == -1)
	{
		res = mkfifo(fifo_name, 0777);
		if(res != 0)
		{
			fprintf(stderr, "Could not create fifo %s\n", fifo_name);
			exit(EXIT_FAILURE);
		}
	}
	/*以只写阻塞方式打开FIFO文件*/
	pipe_fd_wr = open(fifo_name, open_mode);

	if(pipe_fd_wr == -1)    
		cout << " open iFlytek_2_ros err !" << endl;
	else 
		cout << "open iFlytek_2_ros successfully!" << endl;
	/*******************************************************************/
#endif


	while(1){

		// cin >> choice;

		//default choice: multi-thread
		choice = "multithread";
	

		if(choice=="c"){
			cout << "createAgent" << endl;
			KEVIN.createAgent();
		}
		else if(choice=="w"){
			cout << "wakeup" << endl;
			KEVIN.wakeup();
		}
		else if(choice=="speak"){
			cout << "SpeechRecognition\n" << endl;
			/*Reconition result is saved in  g_result */
			SpeechRecognition();
		}
		else if(choice=="wrt"){
			cout << "writeText to Cloud" << endl;
			KEVIN.writeText(txt2); //g_result
		}
		else if(choice=="syn"){
			cout << "SpeechSynthesis" << endl;
			SpeechSynthesis((char*)ExtractResult.data());   
		}
		else if(choice=="play"){
			/*filepath of the audio to be played*/  
			playSound(wavpath);   
		}
		else if(choice=="demo"){

			cout << "createAgent" << endl;
			KEVIN.createAgent();	

			cout << "wakeup" << endl;
			KEVIN.wakeup();	

			while(1){

				string cmd = "working";
				// cout << cmd << endl;

				while(cmd=="working"){

					// g_result = "开始点餐";

					SpeechRecognition();
					KEVIN.writeText("开始点餐");

					/* wait until AIUI result available */
					while(AIUI_Done==false);  

					if( AnswerType==Custom || AnswerType==Turing ){
						SpeechSynthesis((char*)ExtractResult.data());
						playSound(wavpath);
					}
					else if( AnswerType==NoAnswer ){
						/*common voice response, save to local*/
						SpeechSynthesis(Response_2_noAnswer); 
						playSound(wavpath);
					}
					else if( AnswerType==NoSound ){
						/*common voice response, save to local*/
						SpeechSynthesis(Response_2_noSound[1]);;
						playSound(wavpath);
					}
				
				}

			}
			
		}
		else if(choice=="multithread"){
			
			//order function thread
			pthread_t orderId;

			//get and extract Json from ROS
			pthread_t getCmdId;

	        int ret = pthread_create(&orderId, NULL, _order, NULL);
	        if (ret != 0) 
	            printf("order pthread_create error: error_code = %d\n", ret);


	        ret = pthread_create(&getCmdId, NULL, _extractJsonFromROS, NULL);
	        if (ret != 0) 
	            printf("getCmd pthread_create error: error_code = %d\n", ret);

	        while(1);
		}

	}

    pthread_exit(NULL);

	return 0;

}

	
	
	
