//
// Created by Extra Account on 24/02/2025.
//

#include <stdio.h>
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

int main(){
  char morse[10];

  printf("MorseToCharTranslator\n");
  printf("Enter Morse Code: \n");
  scanf("%6s", morse);

 char letter = getletter(morse);

 if(letter == '\0'){
   printf("No morse code found\n");
 } else {
   printf("Morse code is: %c\n", letter);
 }

  return 0;

}