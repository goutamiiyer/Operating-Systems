#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <ctype.h>

FILE *fp;

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

struct _attribute_((_packed_)) DirectoryEntry
{
	char Dir_Name[11];
	unit8_t Dir_Attr;
	unit8_t Unused1[8];
	unit16_t DIR_FirstClusterHigh;
	unit8_t Unused[4];
	unit16_t DIR_FirstClusterLow;
	unit32_t DIR_FileSize;
};

struct DirectoryEntry dir[16];

unit16_t BPB_BytesPerSec;
unit8_t BPB_SecPerClus;
unit16_t BPB_RsvdSecCnt;
unit8_t BPB_NumFATs;
unit32_t BPB_FATSz32;
unit16_t BPB_RootEntCnt;
unit32_t RootClusAddress;
char BS_VolLab[11];

/*
*Function		:nextLb
*Parameters		:The current sector number that points to a block of data
*Returns		:The current sector number that points to the next block of data
*Description	:Finds the sector number of the next block of data given the initial block of data's sector number
*/

int16_t nextLb (unit32_t sector)
{
	unit32_t FATAddress = (BPB_BytesPerSec * BPB_RsvdSecCnt) + (sector*4);
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
	return ((sector - 2) * BPB_BytesPerSec) + (BPB_BytesPerSec * BPB_RsvdSecCnt) + (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec);
}

int main()
{
	fp = fopen( "fat32.img", "r" );
	if( fp = NULL )
	{
		perror("Error opening file: ");
	}
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
	RootClusAddress = (BPB_NumFATs * BPB_FATSz32 * BPB_BytesPerSec) + (BPB_RsvdSecCnt * BPB_BytesPerSec);

	printf("RootClusAddress %x\n", RootClusAddress);

	fseek(fp, RootClusAddress, SEEK_SET);

	fread( &dir[i], sizeof( struct DirectoryEntry), 16, fp);

	int i;
	for( i = 0; i <16; i++ )
	{
		char name[12];
		memcpy( name, dir[i].Dir_Name, 11 );
		name[11] = '\0';
		printf("%s: %d\n", name, dir[i].DIR_FirstClusterLow );
	}
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
	/*
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
