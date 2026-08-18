#include <iostream>
#include <unistd.h>
#include<sys/wait.h>
#include <cassert> 

int main(int argc, char* argv[])
{
	
	pid_t pid = fork(); 

	if(pid < 0)
	{
		std::cerr << "Fork failed";
		return -1; 
	} else if (pid == 0)
	{
		execv(argv[1], argv + 1); 
		std::perror("execv failed");
		return -1; 
	} else
	{
		int status; 
		pid_t exited_pid = waitpid(pid, &status, 0); 
		assert(exited_pid == pid); 
	}

	return 0; 	
}
