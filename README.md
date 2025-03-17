# MorseCodeDriver
Kernel Driver Morse Code Translator
To use the program:
1. make
2. sudo insmod MorseDriver.ko
3. sudo mknod /dev/morse0 c (major number) 0
4. gcc MorseApp.c
5. gcc MorseRead.c -o morseread
6. gcc MorseWrite.c -o morsewrite
7. sudo ./a.out
