#include "tea.h"

#include <stdio.h>

int main(int argc, char** argv)
{
	char* msg   = argv[1];
	char key[17] = "1234567890123456";

	encipher((unsigned long*)msg, (unsigned long*)key, 32); 

	printf("[%d][%d][%d][%d][%d][%d][%d][%d]\n",
		msg[0],
		msg[1],
		msg[2],
		msg[3],
		msg[4],
		msg[5],
		msg[6],
		msg[7]
		);

	return 0;
}

