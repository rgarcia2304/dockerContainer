#include <iostream>
#include <unistd.h>
#include<sys/wait.h>
#include <cassert> 
#include <cstdlib>
#include <string> 
#include <vector> 

constexpr size_t STACK_SIZE = 1024 * 1024; 

int child_main(void* arg)
{
	char** args = static_cast<char**>(arg); 
	std::cout << getpid(); 
	execv(args[0], args);
	return 1; 
}


int main(int argc, char* argv[])
{
	std::vector<char> stack(STACK_SIZE); 

	char *stack_top = stack.data() + STACK_SIZE; 

	pid_t pid = clone(child_main, stack_top, CLONE_NEWPID | SIGCHLD, static_cast<void*>(argv + 1)); 


	if (pid == -1)
	{
		std::perror("Clone Failed"); 
		return EXIT_FAILURE; 
	}

	int status; 
	if (waitpid(pid, &status, 0) == -1)
	{
		std::perror("waitpid failed"); 
	}

	if (WIFEXITED(status))
	{
		std::cout << "child exited with code " << WEXITSTATUS(status) << '\n'; 
	}

	return EXIT_SUCCESS; 
}
