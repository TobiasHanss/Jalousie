#ifndef __MOTOR__
#define __MOTOR__

#include <Arduino.h>
#include "Motor.h"

class CMotor
{
public:
    CMotor(const char * functionName, uint32_t runTime_ms, uint32_t IOUp, uint32_t IODown);

    void begin (void);

    void setPos (uint8_t percent);
    void setAngel (uint8_t percent);
    uint8_t getCurrentPos (void);
    uint8_t getSetPos (void);
    uint8_t getAngel (void);    
    void doHoming(void);
    bool isRunning(void) {return runningDown | runningUp;}
    void setRuntime(uint32_t time_ms);

    void goDownToggle(void);
    void goUpToggle(void);

    const char * getFunctionName(void){return taskName;}

private:
    static void taskHandler (void * ClassPointer);

    void update(void);

    void up();
    void down();
    void stop();

    void printStatus(void);

    uint32_t maxRunTimeTicks;
    int32_t currentPos_Ticks;
    int32_t setPos_Ticks;  
    uint32_t ioUp;
    uint32_t ioDown;
    uint16_t maxAngelRunTimeTicks;
    int16_t currentAngel_Ticks;
    int16_t setAngel_Ticks;

    const char * taskName;
    bool runningUp;
    bool runningDown;

    int32_t restTimeCounter;
};



#endif //__MOTOR__