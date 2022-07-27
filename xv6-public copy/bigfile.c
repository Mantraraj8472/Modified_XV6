#include "user.h"
#include "fcntl.h"
#include "fs.h"

// File for testing bigfile
#define BigFile "FileForTesting.txt"

#define Red "\033[38;5;9m"
#define Green "\033[38;5;46m"
#define Yellow "\033[38;5;11m"
#define Blue "\033[38;5;33m"
#define Magenta "\033[38;5;5m"
#define Cyan "\033[38;5;50m"
#define White "\033[38;5;15m"
#define BrightRed "\033[38;5;1m"
#define Reset "\033[0;0m"
#define LightPink "\033[38;5;133m"
#define LightCyan "\033[38;5;87m"
#define LightOrange "\033[38;5;208m"
#define LightPurple "\033[38;5;183m"
#define LightYellow "\033[38;5;220m"


int main(int argc, char const *argv[])
{
	// File descriptor of BigFileName
	int fd;

	// opening file for writing

	printf(1,"\n\t%sOpening %s for writing...%s\n",LightYellow, BigFile, Reset);
	fd = open(BigFile,O_CREATE | O_WRONLY);
	if(fd == -1) {
		printf(2,"%s not created successfully",BigFile);
		exit();
	}
	printf(1,"\t%s%sopened %ssuccessfully %sfor writing.%s\n",LightPurple, BigFile, Green, LightPurple, Reset);

	char block[BSIZE];

	int numberOfBlocksWritten = 0;
	memset(block,numberOfBlocksWritten,sizeof(block));

	printf(1,"\n\t%sWriting into blocks...%s\n",LightYellow, Reset);

	// While write() succeeds to write, writing to file...
	while(write(fd,block,sizeof(block)) > 0){

		// Number of blocks written so far...
		numberOfBlocksWritten++;

		printf(1,"\r\tNumber of blocks written are: %s%d%s", Blue, numberOfBlocksWritten, Reset);

		// Always writing numberOfBlocksWritten % 32 (data) to BigFile
		memset(block,numberOfBlocksWritten % 32,sizeof(block));
	}
	printf(1,"\n");

	// closing file which is opened for writing
	close(fd);

	int numberOfBlocksRead = 0;

	printf(1,"\t%sWriting into blocks %ssuccessfully %scompleted.\n\n%s", LightOrange, Green, LightOrange, Reset);
	printf(1,"\n\t%sOpening %s for reading...%s\n",LightYellow, BigFile, Reset);

	// opening file in read mode...
	fd = open(BigFile,O_RDONLY);
	if(fd == -1) {
		printf(2,"%s not opened successfully",BigFile);
		exit();
	}

	printf(1,"\t%s%s opened %ssuccessfully%s for reading.%s\n\n", LightPurple, BigFile, Green, LightPurple, Reset);
	printf(1,"\t%sReading from blocks...%s\n",LightYellow, Reset);

	// While we can read, reading...
	// Also verfying that data wrote is correct or not...
	while(read(fd,block,sizeof(block)) > 0) {
		for(int i=0;i<BSIZE;i++) {
			if(block[i] != numberOfBlocksRead % 32){
				printf(1,"Data integrity: %sFailed%s", Red, Reset);
				exit();
			}
		}
		numberOfBlocksRead++;
		printf(1,"\r\tNumber of blocks read are: %s%d%s",Blue, numberOfBlocksRead, Reset);
	}

	printf(1,"\n\t%sReading from blocks%s successfully %scompleted.%s\n\n",LightOrange, Green, LightOrange, Reset);

	printf(1,"\t%sData Written %s==%s Data Read.%s\n",LightPurple, Green, LightPurple, Reset);
	printf(1,"\t%sData integrity: %sPassed%s\n",LightOrange, Green, Reset);

	// Removing file which is created for testing
	printf(1,"\n\t%sRemoving %s...%s\n",LightYellow, BigFile, Reset);
	close(fd);
	if(unlink("FileForTesting.txt") < 0) {
		printf(1, "Removing file %sfailed%s\n",Red, Reset);
		exit();
	}
	printf(1,"\t%s removed %ssuccessfully.%s\n\n",BigFile, Green, Reset);
	printf(1,Reset);
	exit();
}