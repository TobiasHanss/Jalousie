#ifndef __WEBIF__
#define __WEBIF__

#include <SPIFFS.h> 
#include <WebServer.h> 

class WebIf
{
public:
    WebIf(int nPort);

    void begin(void);

private:
    static void taskHandler (void * ClassPointer);

    void onRequestData(void);
    void doHomingPost(void);
    void onConfigPost(void);
    void onMotorsPost(void);
    void onSecure(void);
    void onSetup(void);
    //void onSetupPost(void);
    void listFiles(void);    
    WebServer *m_oServer;

};
#endif
