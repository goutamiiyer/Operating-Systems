#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
int main ()
{
	pid_t child_pid = fork();
	if (child_pid == 0)
	{
		child_pid = fork();
		if( child_pid == 0 )
		{
			printf("Stark\n");
			return 0;
		}
		else
		{
			int status;
			wait(&status);
			printf("Lannister\n");
			return 0;
		}
	}
	else
	{
		int status;
		wait( &status );
		printf("Tyrell\n");
		pid_t pid = fork();
		if( pid != 0 )
		{
			int status;
			wait( &status );
			printf("Greyjoy\n");
		}
		else
		{
			printf("Tully\n");
			return 0;
		}
		exit( 0 );
	}
}
