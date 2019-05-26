/*
Name : Goutami Padmanabhan
UTA ID : 1001669338
*/

#define _GNU_SOURCE

#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#define BPB_BytesPerSec_Offset 11
#define BPB_BytesPerSec_Size 2

#define BPB_SecPerClus_Offset 13
#define BPB_SecPerClus_Size 1

#define BPB_RsvdSecCnt_Offset 14
#define BPB_RsvdSecCnt_Size 2

#define BPB_NumFATs_Offset 16
#define BPB_NumFATs_Size 1

#define BPB_RootEntCnt_Offset 17
#define BPB_RootEntCnt_Size 2

#define BPB_FATSz32_Offset 36
#define BPB_FATSz32_Size 4

#define BS_VolLab_Offset 71
#define BS_VolLab_Size 11

#define WHITESPACE " \t\n"      // We want to split our command line up into tokens
// so we need to define what delimits our tokens.
// In this case  white space
// will separate the tokens on our command line

#define MAX_COMMAND_SIZE 255    // The maximum command-line size

#define MAX_NUM_ARGUMENTS 11     // Mav shell only supports five arguments

FILE *fp;

struct __attribute__((__packed__)) DirectoryEntry
{
	char Dir_Name[11];
	uint8_t Dir_Attr;
	uint8_t Unused1[8];
	uint16_t DIR_FirstClusterHigh;
	uint8_t Unused[4];
	uint16_t DIR_FirstClusterLow;
	uint32_t DIR_FileSize;
};

uint16_t BPB_BytesPerSec;
uint8_t BPB_SecPerClus;
uint16_t BPB_RsvdSecCnt;
uint8_t BPB_NumFATs;
uint32_t BPB_FATSz32;
uint16_t BPB_RootEntCnt;
uint32_t RootClusAddress;
char BS_VolLab[11];

struct DirectoryEntry dir[16];

/*
*Function		:nextLb
*Parameters		:The current sector number that points to a block of data
*Returns		:The current sector number that points to the next block of data
*Description	:Finds the sector number of the next block of data given the initial block of data's sector number
*/


int16_t nextLb (uint32_t sector)
{
	uint32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (sector*4);
	int16_t val;
	fseek(fp, FATAddress, SEEK_SET);
	fread(&val, 2, 1, fp);
	return val;
}


/*
Function	: LBAtoOffset
Parameters	: The current sector number that points to a block of data
Returns		: The value of the address for that block of data
Description	: Finds the starting address of a block of data given the sector number
*corresponding to the data block.
*/

int LBAtoOffset(int32_t sector)
{
	return ((sector - 2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt)
															+ (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

void expandFileName(char* givenName, char* expanded_name) {
	memset( expanded_name, ' ', 12 );
	char *tokenName = strtok(givenName, "." );
	strncpy( expanded_name, tokenName, strlen( tokenName ) );
	tokenName = strtok( NULL, "." );
	if( tokenName )
	{
		strncpy( (char*)(expanded_name+8), tokenName, strlen(tokenName) );
	}
	expanded_name[11] = '\0';
	int j;
	for( j = 0; j < 11; j++ )
	{
		expanded_name[j] = toupper( expanded_name[j] );
	}
}

int main()
{
	char * cmd_str = (char*) malloc( MAX_COMMAND_SIZE );
	fp = NULL;

	while(1) {
		// Print out the msh prompt
		printf ("mfs> ");

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

		//int token_index  = 0;
		//for( token_index = 0; token_index < token_count; token_index ++ )
		//{
		//	printf("token[%d] = %s\n", token_index, token[token_index] );
		//}

		if(token_count > 1 && token[0] != NULL) {
			if(strcmp(token[0], "open") == 0 && token_count == 3) {
				// Open fat32 image
				if(fp != NULL) {
					printf("Error: File system image already open.\n");
				} else {
					fp = fopen(token[1], "r");
					if(fp != NULL) {
						//BPB_BytesPerSec
						fseek(fp, BPB_BytesPerSec_Offset, SEEK_SET);
						fread(&BPB_BytesPerSec, BPB_BytesPerSec_Size, 1, fp);

						//BPB_SecPerClus
						fseek(fp, BPB_SecPerClus_Offset, SEEK_SET);
						fread(&BPB_SecPerClus, BPB_SecPerClus_Size, 1, fp);

						//BPB_RsvdSecCnt
						fseek(fp, BPB_RsvdSecCnt_Offset, SEEK_SET);
						fread(&BPB_RsvdSecCnt, BPB_RootEntCnt_Size, 1, fp);

						//BPB_NumFATs
						fseek(fp, BPB_NumFATs_Offset, SEEK_SET);
						fread(&BPB_NumFATs, BPB_NumFATs_Size, 1, fp);

						//BPB_RootEntCnt
						fseek(fp, BPB_RootEntCnt_Offset, SEEK_SET);
						fread(&BPB_RootEntCnt, BPB_RootEntCnt_Size, 1 ,fp);

						//BPB_FATSz32
						fseek(fp, BPB_FATSz32_Offset, SEEK_SET);
						fread(&BPB_FATSz32, BPB_FATSz32_Size, 1, fp);

						//BS_VolLab
						fseek(fp, BS_VolLab_Offset, SEEK_SET);
						fread(&BS_VolLab, BS_VolLab_Size, 1, fp);

						//Calculating the address of root directory
						RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec)
															+ (BPB_RsvdSecCnt * BPB_BytesPerSec);
						//printf("RootClusAddress %x\n", RootClusAddress);
						fseek(fp, RootClusAddress, SEEK_SET);
						int i;
						for( i = 0; i <16; i++ )
						{
							fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
						}

					} else {
						printf("Error: File system image not found.\n");
					}
				}
			} else if(strcmp(token[0], "close") == 0 && token_count == 2) {
				// Close fat32 image
				if(fp != NULL) {
					fclose(fp);
					fp = NULL;
				} else {
					printf("Error: File system not open.\n");
				}
			} else if(strcmp(token[0], "info") == 0 && token_count == 2) {
				// info
				if(fp != NULL) {
					printf("BPB_BytesPerSec - Hex:%X Base10:%u\n", BPB_BytesPerSec, BPB_BytesPerSec);
					printf("BPB_SecPerClus - Hex:%X Base10:%u\n", BPB_SecPerClus, BPB_SecPerClus);
					printf("BPB_RsvdSecCnt - Hex:%X Base10:%u\n", BPB_RsvdSecCnt, BPB_RsvdSecCnt);
					printf("BPB_NumFATS - Hex:%X Base10:%u\n", BPB_NumFATs, BPB_NumFATs);
					printf("BPB_FATSz32 - Hex:%X Base10:%u\n", BPB_FATSz32, BPB_FATSz32);
				} else {
					printf("Error: File system image must be opened first.\n");
				}
			} else if(strcmp(token[0], "volume") == 0 && token_count == 2) {
				// volume
				if(fp != NULL) {
					printf("Volume Name: %s\n", BS_VolLab);
				} else {
					printf("Error: File system image must be opened first.\n");
				}
			} else if (strcmp(token[0], "stat") == 0 && token_count == 3) {
				char expanded_name[12];
				expandFileName(token[1], expanded_name);
				int i;
				for(i=0; i < 16; i++) {
					char IMG_Name[12];
					memcpy( IMG_Name, dir[i].Dir_Name, 11 );
					IMG_Name[11] = '\0';
					if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
					{
						printf("Name: %s\n",expanded_name);
						printf("Attribute: %u\n",dir[i].Dir_Attr);
						printf("FirstClusterHigh: %u\n",dir[i].DIR_FirstClusterHigh);
						printf("FirstClusterLow: %u\n",dir[i].DIR_FirstClusterLow);
						printf("Size: %u\n",dir[i].DIR_FileSize);
						break;
					}
				}
				if(i == 16) {
					printf("Error: File not found.\n");
				}
			} else if (strcmp(token[0], "get") == 0 && token_count == 3) {
				char expanded_name[12];
				expandFileName(token[1], expanded_name);
				int i;
				for(i=0; i < 16; i++) {
					char IMG_Name[12];
					memcpy( IMG_Name, dir[i].Dir_Name, 11 );
					IMG_Name[11] = '\0';
					if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
					{
						if(dir[i].DIR_FileSize > 0) {
							int file_size = dir[i].DIR_FileSize;
							int LowClusterNumber = dir[i].DIR_FirstClusterLow;
							int offset = LBAtoOffset(LowClusterNumber);
							fseek( fp, offset, SEEK_SET);
							FILE* newfp = fopen(token[1], "w");
							while( file_size >= 512 )
							{
								fwrite( fp, 512, 1, newfp );
								file_size = file_size - 512;
								// find the new logical block
								LowClusterNumber = nextLb(LowClusterNumber);
								if(LowClusterNumber == -1) break;
								offset = LBAtoOffset(LowClusterNumber);
								fseek( fp, offset, SEEK_SET);
							}
							if(file_size > 0) {
								fwrite(fp, file_size, 1, newfp);
							}
							fclose( newfp );
							break;
						}
					}
				}
				if(i == 16) {
					printf("Error: File not found.\n");
				}
			} else if (strcmp(token[0], "ls") == 0 && token_count == 2) {
				int i;
				for(i=0; i < 16; i++) {
					char IMG_Name[12];
					memcpy( IMG_Name, dir[i].Dir_Name, 11 );
					IMG_Name[11] = '\0';
					if(((int)IMG_Name[0]) != 0xE5)
					{
						printf("%s\n",IMG_Name);
					}
				}
			} else if (strcmp(token[0], "cd") == 0 && token_count == 3) {
				if(token[1] == NULL) {
					//Calculating the address of root directory
					RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec)
														+ (BPB_RsvdSecCnt * BPB_BytesPerSec);
					//printf("RootClusAddress %x\n", RootClusAddress);
					fseek(fp, RootClusAddress, SEEK_SET);
					int i;
					for( i = 0; i <16; i++ )
					{
						fread(&dir[i], sizeof(struct DirectoryEntry), 1, fp);
					}
				} else {
					char expanded_name[12];
					expandFileName(token[1], expanded_name);
					int i;
					for(i=0; i < 16; i++) {
						char IMG_Name[12];
						memcpy( IMG_Name, dir[i].Dir_Name, 11 );
						IMG_Name[11] = '\0';
						if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
						{
							int LowClusterNumber = dir[i].DIR_FirstClusterLow;
							int offset = LBAtoOffset(LowClusterNumber);
							fseek(fp, offset , SEEK_SET);
							int j;
							for( j = 0; j < 16; j++ )
							{
								fread(&dir[j], sizeof(struct DirectoryEntry), 1, fp);
							}
							break;
						}
					}
					if(i == 16) {
						printf("Error: Directory not found.\n");
					}
				}
			} else if (strcmp(token[0], "read") == 0 && token_count == 5) {
				char expanded_name[12];
				expandFileName(token[1], expanded_name);
				int position = atoi(token[2]);
				int numBytes = atoi(token[3]);
				int i;
				for(i=0; i < 16; i++) {
					char IMG_Name[12];
					memcpy( IMG_Name, dir[i].Dir_Name, 11 );
					IMG_Name[11] = '\0';
					if( strncmp( expanded_name, IMG_Name, 11 ) == 0 )
					{
						if(dir[i].DIR_FileSize > 0) {
							if(position+numBytes <= dir[i].DIR_FileSize) {
								int file_size = dir[i].DIR_FileSize;
								int LowClusterNumber = dir[i].DIR_FirstClusterLow;
								int offset = LBAtoOffset(LowClusterNumber);
								int currPosition = 0;
								while( file_size >= 512 )
								{
									int j;
									for(j=0; j < 512; j++) {
										if(currPosition >= position && currPosition < (position+numBytes)) {
											fseek(fp, offset+j, SEEK_SET);
											printf("%c",fgetc(fp));
											if(currPosition == position+numBytes-1) {
												printf("\n");
											}
										}
										currPosition++;
									}

									file_size = file_size - 512;
									// find the new logical block
									LowClusterNumber = nextLb(LowClusterNumber);
									if(LowClusterNumber == -1) break;
									offset = LBAtoOffset(LowClusterNumber);
								}
								if(file_size > 0) {
									int j;
									for(j=0; j < file_size; j++) {
										if(currPosition >= position && currPosition < (position+numBytes)) {
											fseek(fp, offset+j, SEEK_SET);
											printf("%c",fgetc(fp));
											if(currPosition == position+numBytes-1) {
												printf("\n");
											}
										}
										currPosition++;
									}
								}
							} else {
								printf("Error: Not all positions in file.\n");
							}
							break;
						}
					}
				}
				if(i == 16) {
					printf("Error: File not found.\n");
				}
			}
		}
		free( working_root );
	}

	/*
	fp = fopen("fat32.img", "r");
	if( fp = NULL )
	{
		perror("Error opening file: ");
	}

	//Calculating the address of root directory
	RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);

	printf("RootClusAddress %x\n", RootClusAddress);

	fseek(fp, RootClusAddress, SEEK_SET);

	fread( &dir[i], sizeof( struct DirectoryEntry), 16, fp);


	// read filename offset number

	// Convert the filename to uppercase AND pad if necessary

	// user input = foo.txt
	//directory = FOO     TXT

	// Lookup in the directory for the file
	// Save the low cluster number
	// Save the file size

	// Call LBAToOffset( low cluster number );
	// fseek to offset
	// fopen the original filename

	int file_size = dir[i].DIR_FileSize;
	int LowClusterNumber = dir[i].DIR_LowClusterNumber;
	int offset = LBAToOffset( LowClusterNumber);
	fseek( fp, offset, SEEK_SET);

	while( file size >= 512 )
	{
		fwrite( newfp, 512, 1, fp );
		file_size = file_size - 512;

		// find the new logical block
		LowClusterNumber = NextLB( LowClusterNumber );

		if( LowClusterNumber = -1) break;

		offset = LBAToOffset( LowClusterNumber )
		fseek( fp, offset, SEEK_SET);
	}

	if( file_size > 0)
		fwrite( newfp, file_size, 1, fp );

	fclose( newfp )
	*/
	// while( file size > 512 low cluster number != -1)
	// fwrite to the new fp 512 bytes
	// subtract 512 from file size
	// NextLB( with low cluster number )
	// Call LBAToOffset
	// If offset != -1 fseek
	// else break;

	// fwrite remaining file size to file

	return 0;
}
