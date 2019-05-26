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

typedef struct
{
		int pid_number;
		char *cmd_token[MAX_NUM_ARGUMENTS];
		int num_tokens;
} Process;

int main()
{

		char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );

		//int count = 0;
		Process arr_process[15];
		int i;
		for(i=0; i < 15; i++) {
				arr_process[i].pid_number = -1;
		}
		int arr_head = 0;
		//int current = arr_head;
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

				// int token_index  = 0;
				// for( token_index = 0; token_index < token_count; token_index ++ ) 
				// {
				//   printf("token[%d] = %s\n", token_index, token[token_index] );  
				// }


				// Now print the tokenized input as a debug check
				// \TODO Remove this code and replace with your shell functionality


				pid_t pid = 100;

				if(strstr(cmd_str,"listpids") == NULL && strstr(cmd_str,"history") == NULL
								&& strcmp(cmd_str,"\n") != 0) {
						pid = fork();
				} 

				if (pid == -1)
				{
						perror("fork failed");
						exit(EXIT_FAILURE);
				}
				else if (pid == 0)
				{
						printf("Hello from the child process\n");

						char cmd_path[100];
						char cmd[100];
						char exit_string[100];

						strcpy(exit_string, token[0]);

						if(strcmp(exit_string,"exit") == 0)
						{
								printf("\nBye\n");
								exit(pid);
						}
						if(strcmp(exit_string,"quit") == 0)
						{
								printf("\nBye\n");
								exit(pid);
						}


						if(strcmp(exit_string,"cd") == 0)
						{
								exit(0);
						}

						int  execCmd = 1;
						
						if(exit_string[0] == '!') {
							int first_digit = exit_string[1] - '0';
							int second_digit = exit_string[2] - '0';
							int reqCmd = first_digit;
							if(second_digit >= 0 && second_digit <= 5) {
								reqCmd = reqCmd*10+second_digit;
							}
							if(reqCmd <= 15 && reqCmd > 0) {
								int j;
								int index = arr_head;
								int validCmd = 0;
								for(j=0; j < 15; j++) {
										if(arr_process[index].pid_number != -1) { 
                			validCmd++;
										}
										if(validCmd == reqCmd) {
											strcpy(token[0],arr_process[index].cmd_token[0]);	
											int tokenid;
											for(tokenid=0; tokenid < MAX_NUM_ARGUMENTS; tokenid++) {
												token[tokenid] = NULL;
											}
											for(tokenid=0; tokenid < arr_process[index].num_tokens; tokenid++) {
												token[tokenid] = strndup(arr_process[index].cmd_token[tokenid],MAX_COMMAND_SIZE );
											}
											break;
										}
										index = (index + 1)%15;
								}
								if(reqCmd > validCmd) {
									printf("Only %d commands in history!\n",validCmd);
									execCmd = 0;
								}	
							} else {
								printf("History is available only for 15 commands..\n");
								execCmd = 0;
							} 
						}

						if(execCmd == 1) {
							printf("the command is %s\n",token[0]);
							
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
				else
				{

						if(strcmp(cmd_str,"\n") != 0 )
						{
								printf("Hello from the parent process\n");

								if(strcmp(token[0],"listpids") == 0 || strcmp(token[0],"history") == 0)
								{
									int j;
									int index = arr_head;
									int valid_id = 0;
									for(j=0; j < 15; j++) {
													if(arr_process[index].pid_number != -1) { 
																	valid_id++;
																	if (strcmp(token[0],"listpids") == 0)
																	{
																					printf("%d.%d\n",valid_id,arr_process[index].pid_number);
																	}
																	else
																	{
																		int k;
																		printf("%d.",valid_id);
																		for(k=0; k < arr_process[index].num_tokens; k++) {
																					printf("%s ",arr_process[index].cmd_token[k]);
																		}
																		printf("\n");
																	}
													}          
													index = (index + 1)%15;
									}
									arr_process[arr_head].num_tokens = 0;
									int tokenid;
									for(tokenid=0; tokenid < token_count; tokenid++) {
										if(token[tokenid] != NULL) {
											arr_process[arr_head].cmd_token[tokenid] = strndup( token[tokenid], MAX_COMMAND_SIZE );
											arr_process[arr_head].num_tokens++;
										}
									}
									arr_process[arr_head].pid_number = pid;
									arr_head = (arr_head + 1)%15;
								} else if(token[0][0] == '!') {
									int status;
									waitpid(pid, &status, 0 );
									if (status == 0 && (strcmp(token[0],"exit") == 0 || strcmp(token[0],"quit") == 0))
									{
											exit(0);
									}
								} else {
									int status;

									arr_process[arr_head].num_tokens = 0;
									int tokenid;
									for(tokenid=0; tokenid < token_count; tokenid++) {
										if(token[tokenid] != NULL) {
											arr_process[arr_head].cmd_token[tokenid] = strndup( token[tokenid], MAX_COMMAND_SIZE );
											arr_process[arr_head].num_tokens++;
										}
									}
									arr_process[arr_head].pid_number = pid;
									arr_head = (arr_head + 1)%15;

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

