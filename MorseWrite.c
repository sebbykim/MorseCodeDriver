#include <stdio.h>		// gives us input/output functions like printf, scanf, putchar
#include <unistd.h>		// gives us standared UNIX functions like read, close - for file descriptor operations
#include <fcntl.h>		// gives file control functions like open
#include <string.h>		// for string manipulation like strcmp, strtok
#include <sys/ioctl.h>	// allows us to send commands to the driver
#include <ctype.h>		// gives us tolower() function
#include "ioctl.h"		// custom file used to change the timings 


struct MorseCodeMap{
	char character;
	char *morse;
	};
  
  struct MorseCodeMap mappings[] = {	//an array of MorseCodeMap structs
	  {'a', ".-"},				//maps a to .- 
	  {'b', "-..."},
	  {'c', "-.-."},
	  {'d', "-.."},
	  {'e', "."},
	  {'f', "..-."},
	  {'g', "--."},
	  {'h', "...."},
	  {'i', ".."},
	  {'j', ".---"},
	  {'k', "-.-"},
	  {'l', ".-.."},
	  {'m', "--"},
	  {'n', "-."},
	  {'o', "---"},
	  {'p', ".--."},
	  {'q', "--.-"},
	  {'r', ".-."},
	  {'s', "..."},
	  {'t', "-"},
	  {'u', "..-"},
	  {'v', "...-"},
	  {'w', ".--"},
	  {'x', "-..-"},
	  {'y', "-.--"},
	  {'z', "--.."},
	  {'1', ".----"},
	  {'2', "..---"},
	  {'3', "...--"},
	  {'4', "....-"},
	  {'5', "....."},
	  {'6', "-...."},
	  {'7', "--..."},
	  {'8', "---.."},
	  {'9', "----."},
	  {'0', "-----"},
	  {'.', ".-.-.-"},
	  {',', "--..--"}
  };

  const char *getMorse(char character) {	//using character, finds associated morse code
    int length = sizeof(mappings) / sizeof(mappings[0]); //iterates throught mappings[] to attempt to find character provided
    for (int i = 0; i < length; i++) { 
        if (mappings[i].character == character) {
            return mappings[i].morse;
        }
    }
    return NULL;
}


int main() {
	int fd;
	char char_array[256];
	char morse_array[1024] = "";
	int c;

	fd = open("/dev/morse0", O_RDWR); //opens 
	if (fd < 0) {					  //error handling for failure to open file
		perror("open");
		return fd;
	}

	printf("CharToMorse Translator\n");
	printf("Enter text to be translated:");


	fgets(char_array, sizeof(char_array), stdin); // scans entire user input
	char_array[strcspn(char_array, "\n")] = '\0'; // removes new line from array

	
	int charLength = strlen(char_array); 				//initialize length of the array for iteration
	for(int i = 0; i<charLength; i++) {					//iterate through array
		char_array[i] = tolower(char_array[i]); 		//convert any capital letters ot lowercase
		if(char_array[i] != ' ') {						//if character is not a space
			const char *add = getMorse(char_array[i]);  //add the morse counterpart of the character to a new pointer add
			if(add == NULL){							
				printf("ERROR: UNRECOGNISED CHARACTER\n"); //print an error if the character is not a space or a character in mappings[]
				return 0;
			}
			strcat(morse_array, add); 			//concatenate the morse string in add to the morse array
			strcat(morse_array, "/");			//concatenate a slash to signify the gap in letters
		}
		else { 
			strcat(morse_array, " ");			//if character is a space, add a space to the array
		}
	}
	int morseLength = strlen(morse_array); 		//initialize length of array for write function
	write(fd,morse_array,morseLength);			//write function to kernel space (MorseDriver.c) for led to light
	printf("%s\n", morse_array);				//also prints out final morse array
    printf("Press any key to exit.");			//provide an exit buffer to finish forked process
    getchar();												
	return 0;
}
