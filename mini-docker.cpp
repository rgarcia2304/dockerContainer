#include <iostream>
#include <unistd.h>
#include<sys/wait.h>
#include <cassert> 
#include <cstdlib>
#include <string> 
#include <vector> 
#include <sys/mount.h>
#include <cstring>
#include <sys/syscall.h>
#include <sys/stat.h>

constexpr size_t STACK_SIZE = 1024 * 1024; 

int child_main(void* arg)
{
	char** args = static_cast<char**>(arg); 
	const char* host = "mini-docker";

	bool bind_mounted = false; 
	bool pivoted = false;

	const char* new_root = "/home/cmpsc311/mini-rootfs";
	std::string old_root_path = std::string(new_root) + "/oldrootfs";

	if(sethostname(host, strlen(host)) == -1)
	{
		std::perror("set hostname failed");
	}
	
	if(mount(nullptr, "/", nullptr, MS_REC | MS_PRIVATE, nullptr) == -1)
	{
		std::perror("private remount failed"); 
		goto fail; 
	}

	if(mount(new_root, new_root, nullptr, MS_BIND, nullptr) == -1)
	{
		std::perror("Failure to mount root to itself"); 
		goto fail; 
	}
	
	bind_mounted = true; 
	if(mkdir(old_root_path.c_str(), 0777) == -1)
	{
		std::perror("Failure to create old root path for destruction");
		return -1; 
	}

	if(syscall(SYS_pivot_root, new_root, old_root_path.c_str()) == -1)
	{
		std::perror("Failure to pivot root"); 
		return -1; 
	}
	pivoted = true; 

	if(chdir("/") == -1)
	{
		std::perror("Failure to change directories"); 
		return -1; 
	}

	if(umount2("/oldrootfs", MNT_DETACH) == -1)
	{
		std::perror("Failure to unmount oldfs"); 
		return -1; 
	}

	if(rmdir("/oldrootfs") == -1)
	{
		perror("failure to change directories"); 
	}

	mount("proc", "/proc", "proc", 0, nullptr); 
	std::cout << getpid() << std::endl; 
	execv(args[0], args);
	std::perror("execv failed");

	fail: 
		if(bind_mounted)
		{
			if(umount2(new_root, MNT_DETACH) == -1)
			{
				perror("failure to unmount bind"); 
				 
			}
		}

		if(umount2("/", MNT_DETACH) == -1)
		{
			perror("failure to unmount private");  
		}

		return -1; 
	return 1; 
}


int main(int argc, char* argv[])
{
	std::vector<char> stack(STACK_SIZE); 

	char *stack_top = stack.data() + STACK_SIZE; 

	pid_t pid = clone(child_main, stack_top, CLONE_NEWUTS |  CLONE_NEWNS | CLONE_NEWPID | SIGCHLD, static_cast<void*>(argv + 1)); 


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
