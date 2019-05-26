/*
	Name: Goutami Padmanabhan
	ID  : 1001669338
*/
// The MIT License (MIT)
// 
// Copyright (c) 2016, 2017 Trevor Bakker 
// 
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports five arguments

/*Defining a struct in order to populate the process ids for listpids and command names for history*/
typedef struct
{
		int pid_number;
		char *cmd_token[MAX_NUM_ARGUMENTS];
		int num_tokens;
} Process;

int main()
{

		char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

		Process arr_process[15];
		int i;
		/*Initializing the process ID since only 1 to 15 process IDs have to be printed*/
		for(i=0; i < 15; i++) {
				arr_process[i].pid_number = -1;
		}
		int arr_head = 0;
		int cd_flag = 0;

		while( 1 )
		{
				// Print out the msh prompt
				printf ("msh> ");

				// Read the command from the commandline.  The
				// maximum command that will be read is MAX_COMMAND_SIZE
				// This while command will wait here until the user
				// inputs something since fgets returns NULL when there
				// is no input
				while( !fgets (cmd_str, MAX_COMMAND_SIZE, stdin) );

				/* Parse input */
				char *token[MAX_NUM_ARGUMENTS];

				int   token_count = 0;

				// Pointer to point to the token
				// parsed by strsep
				char *arg_ptr;                                         

				char *working_str  = strdup( cmd_str );                

				// we are going to move the working_str pointer so
				// keep track of its original value so we can deallocate
				// the correct amount at the end
				char *working_root = working_str;

				// Tokenize the input stringswith whitespace used as the delimiter
				while ( ( (arg_ptr = strsep(&working_str, WHITESPACE ) ) != NULL) && 
								(token_count<MAX_NUM_ARGUMENTS))
				{
						token[token_count] = strndup( arg_ptr, MAX_COMMAND_SIZE );
						if( strlen( token[token_count] ) == 0 )
						{
								token[token_count] = NULL;
						}
						token_count++;
				}

				/*Initializing the pid so that it does not fork if there is a lispids command or history command*/
				pid_t pid = 100;

				/*Do not fork for lispids or history command. Otherwise fork the process*/
				if(strstr(cmd_str,"listpids") == NULL && strstr(cmd_str,"history") == NULL
								&& strcmp(cmd_str,"\n") != 0) 
				{
						pid = fork();
				} 

				/*If the fork fails, display error*/
				if (pid == -1)
				{
						perror("fork failed");
						exit(EXIT_FAILURE);
				}
				/*If the fork was successful, child process is created*/
				else if (pid == 0)
				{

						char cmd_path[100];
						char cmd[100];
						char exit_string[100];

						strcpy(exit_string, token[0]);

						/*Shell will exit with status zero if the command is “quit” or “exit”.*/

						if(strcmp(exit_string,"exit") == 0)
						{
								exit(pid);
						}
						if(strcmp(exit_string,"quit") == 0)
						{
								exit(pid);
						}


						if(strcmp(exit_string,"cd") == 0)
						{
								exit(0);
						}

						/*Flag to represent if execvp should be executed or not*/

						int  execCmd = 1;

						/*This functionality is to execute the nth command specified by !n
						Typing !n, where n is a number between 1 and 15 will result in your shell re-running the nth command.
						If the nth command does not exist then your shell will state “Command not in history.”.*/
						if(exit_string[0] == '!') 
						{
							/*This logic is to check each and every digit in !n
							For example, if !n is !14, then first_digit will have 1 and second_digit will have 4*/
							int first_digit = exit_string[1] - '0';
							int second_digit = exit_string[2] - '0';
							int reqCmd = first_digit;
							if(second_digit >= 0 && second_digit <= 5) 
							{
								reqCmd = reqCmd*10+second_digit;
							}
							/*It checks if the number is between 1 and 15*/
							if(reqCmd <= 15 && reqCmd > 0) 
							{
								int j;
								int index = arr_head;
								int validCmd = 0;
								for(j=0; j < 15; j++) 
								{
										/*Executes only if it is a valid process number*/
										if(arr_process[index].pid_number != -1) 
										{ 
                							validCmd++;
										}
										if(validCmd == reqCmd) 
										{
											/*Tokenize the nth command and allocate memory*/
											strcpy(token[0],arr_process[index].cmd_token[0]);	
											int tokenid;
											for(tokenid=0; tokenid < MAX_NUM_ARGUMENTS; tokenid++) 
											{
												token[tokenid] = NULL;
											}
											for(tokenid=0; tokenid < arr_process[index].num_tokens; tokenid++) 
											{
												token[tokenid] = strndup(arr_process[index].cmd_token[tokenid],MAX_COMMAND_SIZE );
											}
											break;
										}
										index = (index + 1)%15;
								}
								/*If the commands enterd is less than the !n, then display error
								For example, if there are only 4 commands in history and you are asking for 10th command*/
								if(reqCmd > validCmd) 
								{
									printf("Only %d commands in history!\n",validCmd);
									execCmd = 0;
								}	
							} 
							else 
								/*If the nth command does not exist then your shell will state “Command not in history.”.*/
							{
								printf("Command not in history.\n");
								execCmd = 0;
							} 
						}

						/*If it is a command other than listpids or history or any function written by this program then check in directories*/

						if(execCmd == 1) 
						{
							/*Any command in /bin, /usr/bin/, /usr/local/bin/ and the current working directory 
							is to be considered valid*/

							/*Shell searches in the following PATH order:1. Current working directory,2. /usr/local/bin
							3. /usr/bin,4. /bin*/
							
							strcpy(cmd_path, "");

							strcpy(cmd, token[0]);
							strcat(cmd_path,cmd);

							execv(cmd_path, token);


							strcpy(cmd_path, "/usr/local/bin/");

							strcpy(cmd, token[0]);
							strcat(cmd_path,cmd);

							execv(cmd_path, token);

							strcpy(cmd_path, "/usr/bin/");

							strcpy(cmd, token[0]);
							strcat(cmd_path,cmd);

							execv(cmd_path, token);


							strcpy(cmd_path, "/bin/");

							strcpy(cmd, token[0]);
							strcat(cmd_path,cmd);

							execv(cmd_path, token);
							printf("%s: Command not found.\n", token[0]);
						}

						fflush(NULL);
						exit(1);
				}
				/*Once child process finishes, parent process is executed*/
				else
				{

						if(strcmp(cmd_str,"\n") != 0 )
						{
								/*This loogic executes lispids and history.
								It uses a circular array to store the previous command and process ids.
								As the user enters the commands, the pids and commands are stored one after another.
								If the command, exceeds more than 15, then every time the user enters a new command, 
								that command overwrites the oldest command.
								arr_head points to the oldest command entered*/
								if(strcmp(token[0],"listpids") == 0 || strcmp(token[0],"history") == 0)
								{
									int j;
									int index = arr_head;
									int valid_id = 0;
									for(j=0; j < 15; j++) {
													if(arr_process[index].pid_number != -1) 
													{ 
																	valid_id++;
																	if (strcmp(token[0],"listpids") == 0)
																	{
																					printf("%d.%d\n",valid_id,arr_process[index].pid_number);
																	}
																	else
																	{
																		int k;
																		printf("%d.",valid_id);
																		for(k=0; k < arr_process[index].num_tokens; k++) 
																		{
																					printf("%s ",arr_process[index].cmd_token[k]);
																		}
																		printf("\n");
																	}
													}          
													index = (index + 1)%15;
									}
									arr_process[arr_head].num_tokens = 0;
									int tokenid;
									for(tokenid=0; tokenid < token_count; tokenid++) 
									{
										if(token[tokenid] != NULL) 
										{
											arr_process[arr_head].cmd_token[tokenid] = strndup( token[tokenid], MAX_COMMAND_SIZE );
											arr_process[arr_head].num_tokens++;
										}
									}
									arr_process[arr_head].pid_number = pid;
									arr_head = (arr_head + 1)%15;
								} else if(token[0][0] == '!') 
								{
									int status;
									waitpid(pid, &status, 0 );
									/*Exit functionality is executed with status 0*/
									if (status == 0 && (strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0))
									{
											exit(0);
									}
								} else {
									int status;

									arr_process[arr_head].num_tokens = 0;
									int tokenid;
									for(tokenid=0; tokenid < token_count; tokenid++) 
									{
										if(token[tokenid] != NULL) 
										{
											arr_process[arr_head].cmd_token[tokenid] = strndup( token[tokenid], MAX_COMMAND_SIZE );
											arr_process[arr_head].num_tokens++;
										}
									}
									arr_process[arr_head].pid_number = pid;
									arr_head = (arr_head + 1)%15;

									/*This logic executes the cd .. command. It goes to the previous folder evrytime the user enters cd ..*/

									if(strcmp(token[0],"cd") == 0)
									{
											if (token[1] != NULL)
											{
													cd_flag = chdir(token[1]);
													if(cd_flag == -1)
													{
															printf("%s: No such file or directory", token[1]);
													}
											}
									}

									waitpid(pid, &status, 0 );
									if (status == 0 && (strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0))
									{
											exit(0);
									}

								}
								fflush( NULL );
						}
				}

				free( working_root );

		}
		return 0;
}

