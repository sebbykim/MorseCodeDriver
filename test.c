#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>



typedef struct{
	char character;
	char *morse;
	}MorseCodeMap;
  
  MorseCodeMap mappings[] = {
	  {'a', ".-"},
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

  char getletter(const char *morse){
	for(int i = 0; i < 38; i++){
	  if(strcmp(morse, mappings[i].morse) == 0){
		return mappings[i].character;
	  }
	}
	return '\0';
  }
int main() 
{
	//int option;
	//printf("Morse code application now running\n");
	//printf("\n");
	//printf("Select from the following:\n");
	//printf("1: read morse code input\n");
	//printf("2: write morse code out\n");

	//switch (option){
		//case 1
	//}
	//return Morseread();
	int fd;
	char c;
	char ch;
	char morse[256];
	int i = 0;
	int j = 0;
	int k = 0;

  printf("MorseToCharTranslator\n");
  printf("Enter Morse Code, press ENTER when finished. \n");
  
	fd = open("/dev/hello1", O_RDWR);
	if (fd < 0) {
		perror("open");
		return fd;
	}
scanf("%c", &ch);
if (ch == '\n') {
	while (read(fd, &c, 1)){ 
		morse[i] = c;
		i++;
	}
		morse[i] = '\0';
		printf("%s\n", morse);


 	char *spacetok = strtok(morse, " ");
 	char word[256];
	while (spacetok != NULL) 
{
	char *slashtok = strtok(spacetok, "/");
 	while (slashtok != NULL)
 	{
		char letter = getletter(slashtok);	
		word[j] = letter;
		j++;
		slashtok = strtok(NULL, "/"); 
 	}
	word[j] = ' ';
	j++;
	spacetok = strtok(NULL, " ");
}
word [j] = '\0';
 	if(word[0] == '\0'){
   printf("No morse code found\n");
 	} else {
   printf("Morse code is: %s\n", word);
 	}
}	


	close(fd);
	return 0;
}


 int Morseread(){
	int fd;
	char c;
	fd = open("/dev/hello1", O_RDWR);

	if (fd < 0) {
		perror("open");
		return fd;
	}

	while (read(fd, &c, 1)) 
		putchar(c);

	close(fd);
}
