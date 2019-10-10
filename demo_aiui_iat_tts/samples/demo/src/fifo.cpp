

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <limits.h>
#include <string.h>
#include "fifo.h"

using namespace std;

const char *fifo_name = "/home/kevin/myfifo/fifo";
int pipe_fd = -1;
int res = 0;
int open_mode = O_RDONLY;
char buffer[PIPE_BUF + 1];
int bytes_read = 0;

void Read_FIFO(void){

	//Empty the buffer
	memset(buffer, '\0', sizeof(buffer));
 
	// cout << "Process " << getpid() <<  " opening FIFO O_RDONLY" << endl;
	//以只读阻塞方式打开管道文件，注意与fifowrite.c文件中的FIFO同名
	pipe_fd = open(fifo_name, open_mode);
	
	// cout << "Process " << getpid() <<  " result " <<  pipe_fd << endl;
 
	if(pipe_fd != -1)
	{
		do
		{
			// res返回的实际读取到的字节数
			res = read(pipe_fd, buffer, PIPE_BUF);

		}while(res > 0);

		close(pipe_fd);
		
	}
	// else
	// 	 exit(EXIT_FAILURE);
 

	cout << "Data from FIFO:" << "[ " << buffer << " ]" << endl;
}
