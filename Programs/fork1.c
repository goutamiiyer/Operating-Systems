#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
int main ()
{
	int status;
	int child_pid = fork();
	if( child_pid == 0 )
	{
		printf("child_pid:%d\n",&child_pid);	
		wait(&status);
		printf("Les Miserables\n");
	}
	else
	{
		printf("child_pid !:%d\n",&child_pid);
		wait(&status);
		printf("Hamilton\n");
	}
	int pid = fork();
	if( pid != 0 )
	{
		printf("child_pid !:%d\n",&child_pid);
		printf("pid !:%d\n",&pid);
		wait( &status );
		printf("Rent\n");
		return 0;
	}
	else
	{
		printf("child_pid:%d\n",&child_pid);
		printf("pid:%d\n",&pid);
		wait( &status );
		printf("Wicked\n");
		return 0;
	}
	return 0;
}
