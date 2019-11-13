#ifndef _DEMO_H
#define _DEMO_H

#include <limits.h>


/****** msgFromROS Json format **********
*	{
*	  "command":"idle",
*	  "name":"甘坤",
*	  "gender":"male",
*	  "age":"23",
*	}
*****************************************/
class MsgFromROS
{

public:
	const char* keyCmd    = "\"command\":\""; 
	const char* keyName   = "\"name\":\"";
	const char* keyGender = "\"gender\":\"";
	const char* keyAge    = "\"age\":\"";

	char cmd[20]    = "idle";
	char name[20]   = "unknown";
	char gender[20] = "unknown";
	char age[20]    = "unknown";	
	char Json[PIPE_BUF+1] = "";

	void extract_Json(const char* key, char* Json, char* dest);
};



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
class MsgToROS
{
public:

	string name = "unknown";
	string emotion;
	bool greetingDone     = false;
	string speechRecoResult = "";

	//orderInfo:
	string DrinkName = "unknown";
	string CupNum    = "unknown";
	string CupType   = "unknown";
	string Temp      = "unknown";
	bool OrderFinish = false;

	char Json[PIPE_BUF+1] = "";

	void generate_Json(string name, string emotion, bool greetingDone,string speechRecoResult, string DrinkName, string CupNum, string CupType, string Temp, bool OrderFinish, char* destination);

};


#endif