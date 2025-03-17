#include <stdio.h>		// gives us input/output functions like printf, scanf, putchar
#include <unistd.h>		// gives us standared UNIX functions like read, close - for file descriptor operations
#include <fcntl.h>		// gives file control functions like open
#include <string.h>		// for string manipulation like strcmp, strtok
#include <sys/ioctl.h>	// allows us to send commands to the driver
#include "ioctl.h"		//custom file used to change the timings 



typedef struct{
	char character;
	char *morse;
	}MorseCodeMap;
  
  MorseCodeMap mappings[] = {	//an array of MorseCodeMap structs
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

  const char *getMorse(char character) {	//using character, finds associated morse code
    int length = sizeof(mappings) / sizeof(mappings[0]);
    for (int i = 0; i < length; i++) {
        if (mappings[i].character == character) {
            return mappings[i].morse;
        }
    }
    return NULL;
}



 int morseread(){
	int fd;
	char c;
	char ch;
	char morse[256];
	int i = 0;
	int j = 0;

  printf("MorseToCharTranslator\n");
  printf("Enter Morse Code, press ENTER when finished. \n");
  
	fd = open("/dev/hello1", O_RDWR);
	if (fd < 0) {
		perror("open");
		return fd;
	}
	read(fd,&c,1);
	
	ch = getchar();
	
		read(fd,&c,1);
		while (read(fd, &c, 1)){ 
		morse[i] = c;
		i++;
		}

		morse[i] = '\0';
		printf("%s\n", morse);

		char *spacetok, *slashtok, *saveptr1, *saveptr2;

		spacetok = strtok_r(morse, " ", &saveptr1);
		char word[256];
		while (spacetok != NULL) 
		{
			slashtok = strtok_r(spacetok, "/", &saveptr2);
			while (slashtok != NULL)
			{
				char letter = getLetter(slashtok);	
				word[j] = letter;
				j++;
				slashtok = strtok_r(NULL, "/", &saveptr2); 
			}
			word[j] = ' ';
			j++;
			spacetok = strtok_r(NULL, " ", &saveptr1);
		}

		word [j] = '\0';
		if(word[0] == '\0'){
			printf("No morse code found\n");
		} else {
			printf("Morse code is: %s\n", word);
		}
		


	close(fd);
	return 0;
}

int morsewrite() {
	int fd;
	char char_array[256];
	char morse_array[1024] = "";
	int c;

	fd = open("/dev/hello1", O_RDWR);
	if (fd < 0) {
		perror("open");
		return fd;
	}

	printf("CharToMorse Translator\n");
	printf("Enter text to be translated:");


	fgets(char_array, sizeof(char_array), stdin); // scans entire user input
	char_array[strcspn(char_array, "\n")] = '\0'; // removes new line from array
	
	int charLength = strlen(char_array);
	for(int i = 0; i<charLength; i++) {
		if(char_array[i] != ' ') {
			const char *add = getMorse(char_array[i]);
			strcat(morse_array, add);
			strcat(morse_array, "/");
		}
		else { 
			strcat(morse_array, " ");
		}
	}
	int morseLength = strlen(morse_array);
	write(fd,&morse_array,1);
	printf("%s\n", morse_array);	
}


void settings(){

	int time_for_space;
	int time_for_char;
	int time_for_long;

	int dev = open("/dev/hello1", O_WRONLY);
	if(dev == -1){
		printf("Could not open");
	}

	int speedSetting;
	printf("Select speed mode:\n");
	printf("1: Slow\n");
	printf("2: Medium\n");
	printf("3: Fast\n");
	scanf("%d", &speedSetting);
	switch (speedSetting){
		case 1:
		time_for_space = 3000;
		time_for_char = 1000;
		time_for_long = 500;

		ioctl(dev, SET_TIME_SPACE, &time_for_space);
		ioctl(dev, SET_TIME_CHAR, &time_for_char);
		ioctl(dev, SET_TIME_LONG, &time_for_long);

		printf("Time for space: %dms\n", time_for_space);
		printf("Time in between characters: %dms\n", time_for_char);
		printf("Time for for a long press: %dms\n", time_for_long);
		break;

		case 2:
		time_for_space = 2000;
		time_for_char = 800;
		time_for_long = 400;

		ioctl(dev, SET_TIME_SPACE, &time_for_space);
		ioctl(dev, SET_TIME_CHAR, &time_for_char);
		ioctl(dev, SET_TIME_LONG, &time_for_long);

		printf("Time for space: %dms\n", time_for_space);
		printf("Time in between characters: %dms\n", time_for_char);
		printf("Time for for a long press: %dms\n", time_for_long);
		break;

		case 3:
		time_for_space = 1000;
		time_for_char = 600;
		time_for_long = 300;

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
	int option;
	printf("Morse code application now running\n");
	printf("\n");
	printf("Select from the following:\n");
	printf("1: Read morse code input\n");
	printf("2: Write morse code out\n");
	printf("3: Settings\n");
	scanf("%d", &option);

	while (getchar() != '\n');

	switch (option){
		case 1: 
		morseread();
		break;
		case 2:
		morsewrite();
		break;
		case 3:
		settings();
		break;
	}
}
