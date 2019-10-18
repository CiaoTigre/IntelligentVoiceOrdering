#ifndef _DEMO_H
#define _DEMO_H

#include <limits.h>


class MsgFromROS
{

public:
	const char* keyCmd    = "\"command\":\""; 
	const char* keyName   = "\"name\":\"";
	const char* keyGender = "\"gender\":\"";
	const char* keyAge    = "\"age\":\"";

public:
	char cmd[20]    = "idle";
	char name[20]   = "unknown";
	char gender[20] = "unknown";
	char age[4]     = "00";	
	char Json[PIPE_BUF+1] = "null";

	void extractJsonFromROS(const char* key, char* Json, char* dest);
};




class MsgToROS
{
public:

	char Json[PIPE_BUF+1] = "null";

	void generateJsonToROS(char* name, char* emotion, bool OrderFinish, char* json);

};


#endif