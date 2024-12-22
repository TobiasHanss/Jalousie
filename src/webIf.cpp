#include <Arduino.h>
#include <SPIFFS.h> 
#include <WiFi.h>
#include <ArduinoJson.h>

#include "settings.h"
#include "webIf.h"
#include "task.h"
#include "motor.h"
#include "main.h"

//extern CSettings Settings;
extern CSettings Config;
extern CSettings Secure;

extern CMotor  *Motor[NO_OF_MOTORS];

WebIf::WebIf(int nPort)
{
    m_oServer = new WebServer(nPort);
}

void WebIf::taskHandler (void * ClassPointer)
{
    while(1){
        static_cast<WebIf*> (ClassPointer)->m_oServer->handleClient();
        delay(50);
    }
}

void WebIf::listFiles()
{
    File root = SPIFFS.open("/");
    File file = root.openNextFile();
    Serial.println("FILE   \t\tSize");
    while(file){
        Serial.printf("%s \t%d Byte\n",file.name(),file.size());
        file = root.openNextFile();
    }
}

void WebIf::begin(void)
{

    if (!SPIFFS.begin(true)) {
        Serial.println("SPIFFS init failed");
    }
    
    m_oServer->on("/config.post", HTTP_POST ,std::bind(&WebIf::onConfigPost,this));
    m_oServer->on("/motors.post", HTTP_POST ,std::bind(&WebIf::onMotorsPost,this));
    m_oServer->on("/set",         HTTP_GET ,std::bind(&WebIf::onMotorsPost,this));
    m_oServer->on("/homing.post", HTTP_POST ,std::bind(&WebIf::doHomingPost,this));

    m_oServer->on("/data.json", std::bind(&WebIf::onRequestData,this));
    m_oServer->on("/secure.json", std::bind(&WebIf::onSecure,this));
    m_oServer->serveStatic("/", SPIFFS, "/");

    m_oServer->begin();
    xTaskCreate(taskHandler,"CWebIf",512*6,this,3,NULL );
}

void WebIf::onSetup(void)
{
    const char * redirect =  R"(<!DOCTYPE html><html><head><meta http-equiv="refresh" content="0; url=setup.htm"></head><body><p>Please follow <a href="setup.htm">this link</a>.</p></body></html>)";
    m_oServer->send(200, "text/html", redirect); 
}


void WebIf::onSecure(void)
{
    m_oServer->send(403, "text/plain", "Access denied"); //Send web page
}

void WebIf::doHomingPost(void)
{
    for (int i = 0 ; i < NO_OF_MOTORS;i++ )
    {
        Motor[i]->doHoming();
        delay(100);
    }
    m_oServer->send(200, "text/plain", ""); //Send web page

}

void WebIf::onConfigPost(void)
{
    for (int i = 0 ; i < m_oServer->args();i++ )
    {
        String Value = m_oServer->arg(i);
        if (Value == "off")
            Config.set(m_oServer->argName(i),"on"); 
        else
            Config.set(m_oServer->argName(i),Value);       

        Serial.printf("%s -> %s\n",m_oServer->argName(i),m_oServer->arg(i));
    }
    Config.save();
    m_oServer->send(200, "text/plain", ""); //Send web page


    //Set Runtime
    for (int i = 0 ; i < NO_OF_MOTORS; i++)
    {
        char Txt[] = {"RunTimeMX"}; 
        Txt[8] = '0'+i;
        Motor[i]->setRuntime(Config.getInt(Txt)); 
    }

    //ESP.restart();
}

void WebIf::onMotorsPost(void)
{
    Serial.printf("onMotorsPost() start\n");
    for (int i = 0 ; i < m_oServer->args();i++ )
    {
        uint8_t Value = m_oServer->arg(i).toInt();
        if (m_oServer->argName(i) == "M0set") {Motor[0]->setPos(Value); delay(50);}
        if (m_oServer->argName(i) == "M1set") {Motor[1]->setPos(Value); delay(50);}
        if (m_oServer->argName(i) == "M2set") {Motor[2]->setPos(Value); delay(50);}
        if (m_oServer->argName(i) == "M3set") {Motor[3]->setPos(Value); delay(50);}
        if (m_oServer->argName(i) == "M4set") {Motor[4]->setPos(Value); delay(50);}
        if (m_oServer->argName(i) == "M5set") {Motor[5]->setPos(Value); delay(50);}

        if (m_oServer->argName(i) == "AngelSet") {for (int i = 0 ; i < NO_OF_MOTORS; i++) {Motor[i]->setAngel(Value); delay(50);}}

        Serial.printf("%s -> %s\n",m_oServer->argName(i),m_oServer->arg(i));
    }

    m_oServer->send(200, "text/plain", ""); //Send web page
}

void WebIf::onRequestData(void)
{
    StaticJsonDocument<600> oJSON;    
    static int i = 0 ;

    oJSON["Uptime"] = ((millis()/1000)/60); 
    oJSON["LOCAL_IP"] = WiFi.localIP();
    oJSON["MAC"]  = WiFi.macAddress(); 
    for (int i = 0 ; i < NO_OF_MOTORS; i++)
    {
        String TextA = "M"+ String(i) + "current"; 
        oJSON[TextA] = Motor[i]->getCurrentPos(); 
        String TextB= "M"+ String(i) + "running"; 
        oJSON[TextB] = Motor[i]->isRunning();
        String TextC= "M"+ String(i) + "set"; 
        oJSON[TextC] = Motor[i]->getSetPos();
    }

    oJSON["AngelCurrent"] = Motor[0]->getAngel();
    oJSON["AngelSet"] = Motor[0]->getAngel();
    oJSON["TempSens1"] = String(tempSensor1) + "°C";
    oJSON["Time"] = timeClient.getFormattedTime();

    String buf;
    serializeJson(oJSON,buf);
    m_oServer->send(200, "application/json", buf);
}


