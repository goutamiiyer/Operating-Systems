#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>
int main ()
{
	int status;
	int child_pid = fork();
	printf("child_pid:%d\n", child_pid);
	if( child_pid == 0 )
	{
		wait(&status);
		printf("Les Miserables\n");
	}
	else
	{
		wait(&status);
		printf("Hamilton\n");
	}
	int pid = fork();
	printf("pid:%d\n", pid);
	if( pid != 0 )
	{
		wait( &status );
		printf("Rent\n");
		return 0;
	}
	else
	{
		wait( &status );
		printf("Wicked\n");
		return 0;
	}
	return 0;
}
/*
Les Miserables
Wicked
Rent
Hamilton
Wicked
Rent
*/
/*
child_pid:0
Les Miserables
pid:0
Wicked
pid:0
Rent
child_pid:11217
Hamilton
pid:11217
Wicked
pid:11217
Rent

*/
