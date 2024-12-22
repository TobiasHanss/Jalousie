#ifndef __MAIN__
#define __MAIN__

#include <NTPClient.h>


#define NO_OF_MOTORS 6

#define NO_OF_INPUT 2

#define ALL_UP      34  //Taste 1
#define ALL_DOWN    35  //Taste 2

#define UP1   14
#define DOWN1 12
#define UP2   33
#define DOWN2 25
#define UP3   26
#define DOWN3 27


#define UP4   04
#define DOWN4 16
#define UP5   17
#define DOWN5 05
#define UP6   18
#define DOWN6 19

#define GREEN_LED   32

extern int tempSensor1;
extern NTPClient timeClient;
#endif