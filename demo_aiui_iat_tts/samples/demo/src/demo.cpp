
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

/* extracted from resultChar */
extern string ExtractResult;   
extern bool AIUI_Done;
extern bool OrderFinish;

/* type of answer by AIUI*/
extern ANSWER_TYPE AnswerType;

/* audio file path to be played, depends on the setting of SpeechSynthesis() */
const char* wavpath = "demo.wav";  

const char* Greeting              = "您好，欢迎来到智能吧台，我是大眼睛服务机器人，我将根据您的特征信息推荐一些适合您的饮品，待会您可在平板上查看推荐选项，决定后直接告诉我即可！";
const char* gender[2]             = {"先生",
									 "女士"};


const char* Response_2_noAnswer   = "不好意思，可以告诉我想喝什么吗？";
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

void MsgFromROS::extractJsonFromROS(const char* key, char* json, char* dest){

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


void MsgToROS::generateJsonToROS(char* name, char* emotion, bool OrderFinish, char* json){

	char temp1[512] = "{\"name\":\"";
	char temp2[20]  = "\",\"emotion\":\"";
	char temp3[20]  = "\",\"OrderFinish\":\"";

	strcat(temp1,name);

	strcat(temp1,temp2);

	strcat(temp1,emotion);

	strcat(temp1,temp3);

	OrderFinish == true ? strcat(temp1,"1") : strcat(temp1,"0");

	strcat(temp1,"\"}");

	strcpy(json,temp1);
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
				while(AIUI_Done==false);

				cout << "[ 'order' ] command is currently equal to " << "'"<< msgFromROS.cmd  << "'" << endl;
				
				/*greeting first*/
				/*empty the extStitching*/
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
				msgToROS.generateJsonToROS(msgFromROS.name, "honored", OrderFinish, msgToROS.Json);
				int res = write(pipe_fd_wr, msgToROS.Json, strlen(msgToROS.Json));
				if(res==-1)  cout << "[ 'order' ] write err! " << endl;

				/*synthesis greeting audio & play*/
				SpeechSynthesis(textStitching);
				playSound(wavpath);
			}

			/*check the cmd*/
			if(0!=strcmp(msgFromROS.cmd,"working")) {
				/*specific response here*/
				break;
			}

			/*recording process*/
			SpeechRecognition();

			/*write to cloud*/
			KEVIN.writeText(g_result);

			/*wait until AIUI result available*/
			while(AIUI_Done==false);  

			/*check the cmd*/
			if(0!=strcmp(msgFromROS.cmd,"working")) {
				/*specific response here*/
				break;
			}


			/*AIUI_Done = true*/
			if( AnswerType==Custom || AnswerType==Turing ){
				
				if(OrderFinish==true){
					OrderFinish = false;
					//order finish, change the status to 'idle'
					strcpy(msgFromROS.cmd,"idle");
				}	
				
				SpeechSynthesis((char*)ExtractResult.data());
				playSound(wavpath);
			}
			else if( AnswerType==NoAnswer ){

				msgToROS.generateJsonToROS(msgFromROS.name, "regret", OrderFinish, msgToROS.Json);
				int res = write(pipe_fd_wr, msgToROS.Json, strlen(msgToROS.Json));
				if(res==-1)  cout << "[ 'order' ] write err! " << endl;
				
				/*fixed response, save to local*/
				SpeechSynthesis(Response_2_noAnswer); 
				playSound(wavpath);
			}
			else if( AnswerType==NoSound ){

				msgToROS.generateJsonToROS(msgFromROS.name, "puzzled", OrderFinish, msgToROS.Json);
				int res = write(pipe_fd_wr, msgToROS.Json, strlen(msgToROS.Json));
				if(res==-1)  cout << "[ 'order' ] write err! " << endl;
				
				/*fixed response, save to local*/
				SpeechSynthesis(Response_2_noSound[1]);;
				playSound(wavpath);
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


	const char *fifo_name = "/home/kevin/myfifo/ros_2_iFlytek";
	int open_mode = O_RDONLY;
	char buffer[PIPE_BUF+1];
	int bytes_read = 0; 

	//Empty the buffer
	memset(buffer, '\0', sizeof(buffer));
 
	//RD_ONLY & BLOCKING
	pipe_fd_rd = open(fifo_name, open_mode);
	if(pipe_fd_rd == -1)   cout << " open ros_2_iFlytek err !" << endl;


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

		
		msgFromROS.extractJsonFromROS(msgFromROS.keyCmd, msgFromROS.Json,msgFromROS.cmd);
	
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "command: " << msgFromROS.cmd << endl;
	
		msgFromROS.extractJsonFromROS(msgFromROS.keyName, msgFromROS.Json,msgFromROS.name);
		
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "name: " << msgFromROS.name << endl;
		
		msgFromROS.extractJsonFromROS(msgFromROS.keyGender, msgFromROS.Json,msgFromROS.gender);
		
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "gender: " << msgFromROS.gender << endl;

		msgFromROS.extractJsonFromROS(msgFromROS.keyAge, msgFromROS.Json,msgFromROS.age);
		
		cout << "\t\t\t\t\t\t\t" << "[ 'getJson' ] " << left << setw(10) << "age: " << msgFromROS.age << endl;
		
	}
}


#define toROS

int main()
{

	string choice;
	

	cout << "input choice :" << endl;

#ifdef toROS
	/****************** Create FIFO: WR_ONLY & BLOCKING *****************/
	const char *fifo_name = "/home/kevin/myfifo/iFlytek_2_ros";
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

	//test for playWav



	// while(1){
	// 	/*fixed response, save to local*/
	// 	SpeechSynthesis(Response_2_noSound[0]);;
	// 	playSound(wavpath);
	// 	usleep(1000*10);
		
	// }


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

				// cout << cmd << endl;

				while(msgFromROS.cmd=="working"){

					SpeechRecognition();
					KEVIN.writeText(g_result);

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

		}

		while(1);
		
	}

    pthread_exit(NULL);

	return 0;

}

	
	
	
