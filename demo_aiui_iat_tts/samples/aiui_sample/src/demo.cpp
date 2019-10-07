
#include <iostream>

#include "AIUITest.h"

using namespace std;
using namespace aiui;


int main()
{
	AIUITester KEVIN;

	/* Create agent beforehand */
	KEVIN.createAgent();

	KEVIN.readCmd();
	return 0;
}
