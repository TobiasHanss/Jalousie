#include <Arduino.h>
#include "main.h"
#include <esp_task_wdt.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <ArduinoOTA.h>
#include <WiFiAP.h>
#include "settings.h"
#include "webIf.h"
#include "task.h"
#include "motor.h"
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiUdp.h>

//CSettings Settings("/settings.json",1024);
CSettings Config("/config.json",600);
CSettings Secure("/secret.json",100);

WebIf   WebInterface(80);
CMotor  *Motor[NO_OF_MOTORS];

OneWire oneWire(23); // Bus on Pin23
DallasTemperature sensors(&oneWire);// Pass our oneWire reference to Dallas Temperature.

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP);

bool bInSetupMode = false;
volatile bool bInitDone = false;

unsigned int nextSleep = 0;
unsigned int SleepCounter = 0;
int tempSensor1;

//************************************************************
//************************************************************
void SetupOTA(void)
{
    ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
        SPIFFS.end();
        Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();
}


//************************************************************
//************************************************************

void SetupAP(void)
{
  String sSSID = Config.get("DevName");
  sSSID.replace(" ","");
  char cSSID[12];
  sSSID.toCharArray(cSSID,sizeof(cSSID));
  Serial.printf("AP SSID:%s",cSSID);
  WiFi.softAP(cSSID);
  IPAddress myIP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(myIP);
}


//************************************************************
//************************************************************
void Connect2LocalWifi(void)
{
  WiFi.begin(Config.get("SSID"), Secure.get("PSK"));

  String sDevName = Config.get("DevName");
  sDevName.replace(" ","");
  char DevName[12];
  sDevName.toCharArray(DevName,sizeof(DevName));
  MDNS.begin(DevName);
  
#if 0
  Serial.printf("Connecting to %s...",Config.get("SSID"));
  while(WiFi.status() != WL_CONNECTED) 
  { 
    delay(50);
    Serial.print(".");
  }

  Serial.print(",connected with IP Address: ");
  Serial.println(WiFi.localIP());
#endif
}


//************************************************************
//************************************************************
void setup() 
{
  Serial.begin(115200);
  Serial.println();
  Serial.println();
  Serial.println("Starting... ");
  
  pinMode(GREEN_LED, OUTPUT);  
  pinMode(ALL_UP, INPUT_PULLUP);
  pinMode(ALL_DOWN, INPUT_PULLUP);

  Connect2LocalWifi();
 
  WebInterface.begin();
  SetupOTA();

  Motor[0] = new CMotor("Door",   Config.getInt("RunTimeM0"), UP1, DOWN1);
  Motor[1] = new CMotor("East",   Config.getInt("RunTimeM1"), UP2, DOWN2);
  Motor[2] = new CMotor("SouthL", Config.getInt("RunTimeM2"), UP3, DOWN3);
  Motor[3] = new CMotor("Center", Config.getInt("RunTimeM3"), UP4, DOWN4);
  Motor[4] = new CMotor("SouthR", Config.getInt("RunTimeM4"), UP5, DOWN5);
  Motor[5] = new CMotor("West",   Config.getInt("RunTimeM5"), UP6, DOWN6);

  for (int i = 0 ; i < NO_OF_MOTORS; i++)
  {
    Motor[i]->begin();
  }

  sensors.begin();

  timeClient.begin();
  timeClient.setTimeOffset(60*60); //Add an hour to correct for time zone
}


void handleInput (char c)
{
  static int TestCounter = 0;
  uint8_t OuputPort[] = {UP1,UP2,UP3,UP4,UP6,DOWN1,DOWN2,DOWN3,DOWN4,DOWN5,DOWN6};

  switch (c)
  {
  case ('1'):
    Serial.println("UP1");
    Motor[0]->goUpToggle();
    break;
  case ('q'):
    Serial.println("DOWN1");
    Motor[0]->goDownToggle();
  break; 
  case ('2'):
    Serial.println("UP2");
    Motor[1]->goUpToggle();
    break;
  case ('w'):
    Serial.println("DOWN2");
    Motor[1]->goDownToggle();
    break; 
  case ('3'):
    Serial.println("UP3");
    Motor[2]->goUpToggle();
    break;
  case ('e'):
    Serial.println("DOWN3");
    Motor[2]->goDownToggle();
    break; 
  case ('4'):
    Serial.println("UP4");
    Motor[3]->goUpToggle();
    break;
  case ('r'):
    Serial.println("DOWN4");
    Motor[3]->goDownToggle();
    break;
  case ('5'):
    Serial.println("UP5");
    Motor[4]->goUpToggle();
    break;
  case ('t'):
    Serial.println("DOWN5");
    Motor[4]->goDownToggle();
    break;
  case ('6'):
    Serial.println("UP6");
    Motor[5]->goUpToggle();
    break;
  case ('z'):
    Serial.println("DOWN6");
    Motor[5]->goDownToggle();
    break;
  default:
    Serial.println("unknown key!");
    break;
  }
}


void checkLocalButtons (void)
{
    static bool AllUpOld = false;
    static bool AllDownOld = false;    

    bool AllUp = digitalRead(ALL_UP);
    bool AllDown = digitalRead(ALL_DOWN);

    if (AllUpOld != AllUp & !AllUp)
    {
      for (int i = 0 ; i < NO_OF_MOTORS;i++ )
      {
          Motor[i]->doHoming();
          delay(100);
      }
    }
    else if (AllDownOld != AllDown & !AllDown)
    {
      for (int i = 0 ; i < NO_OF_MOTORS;i++ )
      {
          Motor[i]->setPos(100);
          delay(100);
      }
    }

    AllUpOld = AllUp;
    AllDownOld = AllDown;
}



//************************************************************
//************************************************************
unsigned long reconnectTimer = 10;
unsigned long lastInterval = 0;
unsigned long sensorInterval = 0;
unsigned long ledInterval = 0;
bool Tick;
void loop() 
{ 
  unsigned long now = millis();
  bool WLAN_OK = true;


  if (now - lastInterval > 100) 
  {
    lastInterval = now;

    if(WiFi.status() != WL_CONNECTED) 
    {
      WLAN_OK = false;
      Serial.println("WiFi not connected!");
      WiFi.reconnect();
      if (reconnectTimer < now)
      {
        reconnectTimer = now + 10000;
        WiFi.disconnect();
        WiFi.begin(Config.get("SSID"), Secure.get("PSK"));
      }
    }

    checkLocalButtons();

    while (Serial.available())
    {
      handleInput(Serial.read());
    }

    ArduinoOTA.handle();
  }

  if (now - ledInterval > (WLAN_OK ? 500 : 100)) 
  {
    ledInterval = now;
    Tick = !Tick;
    digitalWrite(GREEN_LED, Tick);
  }

  if (now - sensorInterval > 2000) 
  {
    sensorInterval = now;

    // call sensors.requestTemperatures() to issue a global temperature
    // request to all devices on the bus
    sensors.requestTemperatures(); // Send the command to get temperatures

    if (sensors.getDeviceCount() >= 1)
    {
      tempSensor1 = sensors.getTempCByIndex(0);
    }
    else
    {
      tempSensor1 = -127;
    }

    if (WLAN_OK)
    {
      timeClient.update();
      //Serial.println(timeClient.getFormattedTime());
    }


  }
   delay (20);
}

