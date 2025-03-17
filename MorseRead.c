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

  char getLetter(const char *morse){	//using morse code, finds associated letter
	int length = sizeof(mappings)/sizeof(mappings[0]); //get length of array
	for(int i = 0; i < length; i++){
	  if(strcmp(morse, mappings[i].morse) == 0){ //checks which morse code it matches
		return mappings[i].character;
	  }
	}
	return '\0'; //returns null character if not found
  }


int main(){
	int fd;//stores the file 
	char c;	//stores each morse code
	char morse[256]; //stores all morse code
	int i = 0;//index pointer for morse code
	int j = 0;//index pointer for words

  printf("MorseToCharTranslator\n");
  printf("Enter Morse Code, press ENTER when finished. \n");
  
	fd = open("/dev/morse0", O_RDWR);
	if (fd < 0) {//error handling  for if the file fails to open
		perror("open");
		return fd;
	}
	read(fd,&c,1);
	
	getchar(); //waits for a button press from the user before executing the below code
	
		while (read(fd, &c, 1)){// while there is charaters it appends them to the morse code array 
		morse[i] = c;
		i++;
		}

		morse[i] = '\0';
		printf("%s\n", morse);

		char *spacetok, *slashtok, *saveptr1, *saveptr2; //initializes tokens and associated pointers

		spacetok = strtok_r(morse, " ", &saveptr1); //tokenizes (splits) morse string array into seperate arrays at each space (gaps in words)
		char word[256];
		while (spacetok != NULL) //while there is no space in the array
		{
			slashtok = strtok_r(spacetok, "/", &saveptr2); //tokenizes array based on slashes (gaps in letters)
			while (slashtok != NULL) //while not a slash
			{
				char letter = getLetter(slashtok);	//sends current string of morse values to getLetter(), and assigns it to letter
				word[j] = letter; 					//adds letter to an array, titled word
				j++; 			  					//iterates through array
				slashtok = strtok_r(NULL, "/", &saveptr2); //skips past slash and goes to next string of morse
			}
			word[j] = ' ';   //when a space is encountered, add it to word array to split the word output
			j++;  			 //increments word[] array again
			spacetok = strtok_r(NULL, " ", &saveptr1); //skips past space and continues again
		}

		word [j] = '\0';    
		if(word[0] == '\0'){ 					//if word array is empty
			printf("No morse code found\n");
		} else {
			printf("Morse code is: %s\n", word); //print out morse code
		}

		
        printf("Press any key to exit."); //provide an exit buffer to finish forked process
        getchar(); 						  //wait for key press

	close(fd);
	return 0;
}
