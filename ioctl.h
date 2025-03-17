#ifndef MORSEDRIVER_H
#define MORSEDRIVER_H

//used for ioctl allowing for user applications to set timings for morse code 
#define SET_TIME_CHAR _IOW('m', 'c', int *)
#define SET_TIME_SPACE _IOW('m', 's', int *)
#define SET_TIME_LONG  _IOW('m', 'l', int *)

#endif
