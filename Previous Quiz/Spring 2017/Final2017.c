#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <sys/wait.h>

int main ()
{
	pid_t pid;
	int status;
	pid = fork();
	if (pid == 0)
	{
		wait( &status );
		execl( "/bin/gcc", "gcc", "main.c", NULL );
		printf("Compilation finished\n");
	}
	else
	{
		wait( &status );
		printf("Your executable is compiling.\n");
	}
	wait( &status );
	pid = fork();
	if (pid != 0)
	{
		pid_t cp_pid = fork();
		if( cp_pid )
		{
			wait( &status );
			printf("Copying the file\n");
			execl( "/bin/cp", "cp", "a.out", "main", NULL );
		}
		else
		{
			wait( &status );
			printf("Removing the file\n");
			execl( "/bin/rm", "rm", "main.c", NULL );
		}
		execl( "/bin/rm", "rm", "backup.c", NULL );
	}
	else
	{
		int status;
		wait( &status );
		pid = fork();
		if( !pid )
		{
			wait( &status );
			printf("Done waiting for the child\n");
		}
		else
		{
			wait( &status );
			printf("Done with deliverable\n");
		}
		printf("Build process done!\n");
	}
	printf("Program complete\n");
}
/*
Compilation finished
Done waiting for the child
Build process done!
Program complete
Done with deliverable
Build process done!
Program complete
Removing the file
Copying the file
rm: cannot remove `main.c': No such file or directory
cp: cannot stat `a.out': No such file or directory
Your executable is compiling.
Done waiting for the child
Build process done!
Program complete
Done with deliverable
Build process done!
Program complete
Removing the file
Copying the file
rm: cannot remove `main.c': No such file or directory
cp: cannot stat `a.out': No such file or directory
*/
