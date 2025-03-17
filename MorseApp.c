#include <stdio.h>		// gives us input/output functions like printf, scanf, putchar
#include <unistd.h>		// gives us standared UNIX functions like read, close - for file descriptor operations	
#include <stdlib.h>		//provides exit() function
#include <fcntl.h>		// gives file control functions like open
#include <string.h>		// for string manipulation like strcmp, strtok
#include <sys/ioctl.h>	// allows us to send commands to the driver
#include <ctype.h>		// gives us tolower() function
#include "ioctl.h"		// custom file used to change the timings 



void settings(){

	//initialise timings
	int time_for_space;		//puts a space after waiting for n ms
	int time_for_char;		//separates characters after waiting for n ms
	int time_for_long;		//puts a long dash(-) after n ms, and anything less is a short dash(.)

	//open device file called morse0 for writing
	int dev = open("/dev/morse0", O_WRONLY); //dev is a file descriptor that identifies the device
	if(dev == -1){
		printf("Could not open");
	}

	int speedSetting;

	//menu for setting the speed for the morse code
	printf("Select speed mode:\n");
	printf("1: Slow\n");
	printf("2: Medium\n");
	printf("3: Fast\n");
	scanf("%d", &speedSetting);
	switch (speedSetting){
		case 1:
		//Slow
		//sets times
		time_for_space = 3000;
		time_for_char = 1000;
		time_for_long = 500;

		//sends timings to the device using ioctl functions (through header file)
		ioctl(dev, SET_TIME_SPACE, &time_for_space);
		ioctl(dev, SET_TIME_CHAR, &time_for_char);
		ioctl(dev, SET_TIME_LONG, &time_for_long);

		printf("Time for space: %dms\n", time_for_space);
		printf("Time in between characters: %dms\n", time_for_char);
		printf("Time for for a long press: %dms\n", time_for_long);
		break;

		case 2:
		//Medium
		time_for_space = 2000;
		time_for_char = 800;
		time_for_long = 400;

		//sends timings to the device using ioctl functions (through header file)
		ioctl(dev, SET_TIME_SPACE, &time_for_space);
		ioctl(dev, SET_TIME_CHAR, &time_for_char);
		ioctl(dev, SET_TIME_LONG, &time_for_long);

		printf("Time for space: %dms\n", time_for_space);
		printf("Time in between characters: %dms\n", time_for_char);
		printf("Time for for a long press: %dms\n", time_for_long);
		break;

		case 3:
		//Fast
		time_for_space = 1000;
		time_for_char = 600;
		time_for_long = 300;

		//sends timings to the device using ioctl functions (through header file)
		ioctl(dev, SET_TIME_SPACE, &time_for_space);
		ioctl(dev, SET_TIME_CHAR, &time_for_char);
		ioctl(dev, SET_TIME_LONG, &time_for_long);

		printf("Time for space: %dms\n", time_for_space);
		printf("Time in between characters: %dms\n", time_for_char);
		printf("Time for for a long press: %dms\n", time_for_long);
		break;
	}


}



int main() 
{

	int option; //initialize option for menu
	pid_t pid; //initialize process id for child process

	printf("Morse code application now running\n\n");

	while(option != -1){
	printf("Select from the following:\n");
	printf("1: Read morse code input\n");
	printf("2: Write morse code out\n");
	printf("3: Settings\n");
	printf("-1: Exit\n");
	
	scanf("%d", &option); //scans user input

	while (getchar() != '\n'); //resets buffer

	switch (option){
		case 1: 
		printf("Opening morseread..\n");
		pid = fork(); //creates child process
                
                if (pid < 0) {
                    perror("Fork failed");
                } else if (pid == 0) {
                    execlp("lxterminal", "lxterminal", "-e", "./morseread", NULL); //opens a new terminal window, and executes MorseRead.c

					perror("Exec failed");
					exit(EXIT_FAILURE);
				}
		break;
		case 2:
		printf("Opening morsewrite..\n");
		pid = fork();
                
                if (pid < 0) {
                    perror("Fork failed");
                } else if (pid == 0) {
                    execlp("lxterminal", "lxterminal", "-e", "./morsewrite", NULL); //opens a new terminal window, and executes MorseWrite.c

					perror("Exec failed");
					exit(EXIT_FAILURE);
				}
		break;
		case 3:
		settings(); //executes settings() shown above
		break;
	}
}

}
