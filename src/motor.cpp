#include "motor.h"

#define TimerTick_ms 50
#define RestTimeTick 2 //Ticks


CMotor::CMotor(const char * functionName, uint32_t runTime_ms, uint32_t IODown, uint32_t IOUp)
{
    uint16_t angelRunTime_ms = 1000;

    taskName = functionName;
    maxRunTimeTicks = runTime_ms / TimerTick_ms ;
    currentPos_Ticks = 0;
    setPos_Ticks = 0; 
    maxAngelRunTimeTicks = angelRunTime_ms / TimerTick_ms ;
    currentAngel_Ticks = 0;
    setAngel_Ticks = 0; 
    runningUp = false;
    runningDown = false;
    ioUp = IOUp;
    ioDown = IODown;
    restTimeCounter = 0;

    pinMode(ioUp, OUTPUT);
    pinMode(ioDown, OUTPUT);
    digitalWrite(ioUp,LOW);
    digitalWrite(ioDown,LOW);
}


void CMotor::begin (void)
{
    xTaskCreate(taskHandler,taskName,512*2,this,1,NULL );
}

void CMotor::taskHandler (void * ClassPointer)
{
    Serial.print("Start Task: ");
    Serial.println(static_cast<CMotor*> (ClassPointer)->getFunctionName());
    while(1){
        static_cast<CMotor*> (ClassPointer)->update();
        delay(TimerTick_ms);
    }
}


void CMotor::update(void)
{
    if (currentPos_Ticks > setPos_Ticks)
    {
        down();
        if (currentAngel_Ticks < maxAngelRunTimeTicks)
        {
           currentAngel_Ticks++;
           if (currentAngel_Ticks > maxAngelRunTimeTicks) currentAngel_Ticks = maxAngelRunTimeTicks;
        }
        else
        {
            currentPos_Ticks --; 
            if (currentPos_Ticks < 0) currentPos_Ticks = 0;
        }
    }
    else if (currentPos_Ticks < setPos_Ticks)
    {
        up();
        if (currentAngel_Ticks > 0)
        {
           currentAngel_Ticks--;
           if (currentAngel_Ticks < 0) currentAngel_Ticks = 0;
        }
        else
        {
        currentPos_Ticks ++; 
        if (currentPos_Ticks > maxRunTimeTicks) currentPos_Ticks = maxRunTimeTicks;
        }
    }
    else if (currentAngel_Ticks < setAngel_Ticks)
    {
        down();
        currentAngel_Ticks++;
        if (currentAngel_Ticks > maxAngelRunTimeTicks) currentAngel_Ticks = maxAngelRunTimeTicks;
    }
    else if (currentAngel_Ticks > setAngel_Ticks)
    {
        up();
        currentAngel_Ticks--;
        if (currentAngel_Ticks < 0) currentAngel_Ticks = 0;
    }
    else
    {
        stop();
    }


    if (restTimeCounter > 0) {restTimeCounter --;}
}

void CMotor::up()
{
    
    if (runningDown)
    {
        restTimeCounter = RestTimeTick;
    }
    if (restTimeCounter != 0)
    {
        stop();
        return ;
    }

    digitalWrite(ioUp,HIGH);
    runningUp = true;
/*
    Serial.print(taskName);
    Serial.println(" Up!");
*/

}

void CMotor::down()
{
    if (runningUp)
    {
        restTimeCounter = RestTimeTick;
    }
    if (restTimeCounter != 0)
    {
        stop();
        return ;
    }

    digitalWrite(ioDown,HIGH);
    runningDown = true;
/*
    Serial.print(taskName);
    Serial.println(" Down!");
*/
}

void CMotor::stop()
{
    digitalWrite(ioUp,LOW);
    digitalWrite(ioDown,LOW);
    runningUp = false;
    runningDown = false;

}

void CMotor::setRuntime (uint32_t time_ms)
{
    maxRunTimeTicks = time_ms / TimerTick_ms ;
}

void CMotor::setPos (uint8_t percent)
{
    if ( (percent >= 0) && (percent <= 100) )
    {
        setPos_Ticks = maxRunTimeTicks * percent / 100;
    }
}

void CMotor::setAngel (uint8_t percent)
{
    if ( (percent >= 0) && (percent <= 100) )
    {
        setAngel_Ticks = maxAngelRunTimeTicks * percent / 100;
    }
}


uint8_t CMotor::getCurrentPos (void)
{
    return  currentPos_Ticks * 100 / maxRunTimeTicks; 
}

uint8_t CMotor::getSetPos (void)
{
    return  setPos_Ticks * 100 / maxRunTimeTicks; 
}

uint8_t CMotor::getAngel (void)
{
    return  setAngel_Ticks * 100 / maxAngelRunTimeTicks; 
}



void CMotor::doHoming(void)
{
    currentPos_Ticks = maxRunTimeTicks + (maxRunTimeTicks*10 / 100) ; //maxRun + 10%
    setPos_Ticks = 0;
}

void CMotor::goDownToggle(void)
{
   if ((!runningDown) && (!runningUp))
   {
        setPos(0);
   }
   else
   {
        setPos(getCurrentPos());
   }

    //printStatus();
}

void CMotor::goUpToggle(void)
{
   if ((!runningDown) && (!runningUp))
   {
        setPos(100);
   }
   else
   {
        setPos(getCurrentPos());
   }
    //printStatus();
}

void CMotor::printStatus(void)
{
    Serial.print("isRunning: ");
    Serial.print(isRunning());

    Serial.print(" runningUp: ");
    Serial.print(runningUp);

    Serial.print(" runningDown: ");
    Serial.print(runningDown);

    Serial.print(" restTimeCounter: ");
    Serial.print(restTimeCounter);

    Serial.print(" maxRunTimeTicks: ");
    Serial.print(maxRunTimeTicks);

    Serial.print(" currentPos_Ticks: ");
    Serial.print(currentPos_Ticks);

    Serial.print(" setPos_Ticks: ");
    Serial.println(setPos_Ticks);
}