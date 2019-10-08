
/*
* Description: iFlytek: (iat + aiui + tts) demo  /    ros --> FIFO --> iFlytek
* Author     : Kun Gan
* Date       : Sep. 2019
*/

#include <iostream>
#include "AIUITest.h"
#include "playWav.h"
#include "fifo.h"

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

/* type of answer by AIUI*/
extern ANSWER_TYPE AnswerType;

/* audio file path to be played, depends on the setting of SpeechSynthesis() */
const char* wavpath = "demo.wav";  

const char* Response_2_noAnswer   = "不好意思，可以说的更明白一些吗？";
const char* Response_2_noSound[]  = {"可以说大声一些吗？我好像没有听见你说话。",
									 "朋友，你有在说话吗？",			
									 "一点声音都听不见呀",	
									 "他们太吵了，我都听不清你说的话了"};

string txt1 = "准备点餐！";
string txt2 = "你好呀！";
string txt3 = "今天天气怎样";

int main()
{
	AIUITester KEVIN;

	string cmd;
	
	cout << "input cmd :" << endl;
	
	while(1){

		cin >> cmd;

		if(cmd=="c"){
			cout << "createAgent" << endl;
			KEVIN.createAgent();
		}
		else if(cmd=="w"){
			cout << "wakeup" << endl;
			KEVIN.wakeup();
		}
		else if(cmd=="speak"){
			cout << "SpeechRecognition\n" << endl;
			/*Reconition result is saved in  g_result */
			SpeechRecognition();
		}
		else if(cmd=="wrt"){
			cout << "writeText to Cloud" << endl;
			KEVIN.writeText(txt2); //g_result
		}
		else if(cmd=="syn"){
			cout << "SpeechSynthesis" << endl;
			SpeechSynthesis((char*)ExtractResult.data());   
		}
		else if(cmd=="play"){
			/*filepath of the audio to be played*/  
			playSound(wavpath);   
		}
		else if(cmd=="demo"){

			cout << "createAgent" << endl;
			KEVIN.createAgent();	

			cout << "wakeup" << endl;
			KEVIN.wakeup();				

			while(1){

				SpeechRecognition();
				KEVIN.writeText(g_result);

				/* wait until AIUI result available */
				while(AIUI_Done==false);  

				if( AnswerType==Custom || AnswerType==Turing ){
					SpeechSynthesis((char*)ExtractResult.data());
					playSound(wavpath);
				}
				else if( AnswerType==NoAnswer ){
					//固定回答可以存在本地，节约语音合成的时间
					SpeechSynthesis(Response_2_noAnswer); 
					playSound(wavpath);
				}
				else if( AnswerType==NoSound ){
					//固定回答可以存在本地，节约语音合成的时间
					SpeechSynthesis(Response_2_noSound[1]);;
					playSound(wavpath);
				}
				
			}
		}
			
		//while中循环读取FIFO的数据
		// Read_FIFO();		
	}
		

	return 0;
}

	
	
	
