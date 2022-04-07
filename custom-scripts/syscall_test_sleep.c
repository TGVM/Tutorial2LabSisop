#include <stdio.h>
#include <linux/kernel.h>
#include <sys/syscall.h>
#include <unistd.h>
#include <stdlib.h>

#define SYSCALL_SLEEPPROCESSES	386

int main(int argc, char** argv){  
	char buf[256];
	long ret;
	
	printf("Invoking 'listSleepProcesses' system call.\n");
         
	ret = syscall(SYSCALL_SLEEPPROCESSES, buf, sizeof(buf)); 
         
	if(ret > 0) {
		/* Success, show the process info. */
		printf("%s\n", buf);
	}
	else {
		printf("System call 'listSleepProcesses' did not execute as expected error %d\n", ret);
	}
          
	return 0;
}
