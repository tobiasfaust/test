#ifndef UPDATER_H
#define UPDATER

#include "CommonLibs.h"
#include <vector>
#include "ArduinoJson.h"
#include "_Release.h"

#ifdef ESP8266
   #include <ESP8266httpUpdate.h>
   #include <ESP8266HTTPClient.h>
   #include <WiFiClient.h> // HTTP

   using WM_httpUpdate = ESP8266HTTPUpdate;
   
#elif ESP32
  #include <HTTPClient.h>
  #include <HTTPUpdate.h>
  
  using WM_httpUpdate  = HTTPUpdate;
#endif 

enum stage_t {UNDEF, PROD, PRE, DEV};
  
typedef struct {
    String name;
    String version;
    uint32_t subversion;
    uint32_t number;
    stage_t stage;
    String downloadURL;
  } release_t;

class updater {
  
  public:
    updater();
    void        setIndexJson(String url);
    void        setStage(stage_t s);
    void        setAutoMode(bool a);
    void        setInterval(uint32_t seconds);
    void        loop();
    release_t*  GetCurrentRelease();
    String      GetReleaseName();
    const int&  GetInterval()     const {return interval;}
    std::vector<release_t>* GetReleases();
    void        InstallRelease(uint32_t ReleaseNumber);
    void        RefreshReleases();
    String      GetUpdateErrorString();
    stage_t     String2Stage(String s);
    String      Stage2String(stage_t s);

  private:
    //BearSSL::WiFiClientSecure* client;
    WiFiClient* client;
    WM_httpUpdate* httpUpdate;
    
    void        Update();
    void        downloadJson();
    void        parseJson(String* json);
    //String*     getURLHostName(String* url);
    //String*     getURLPath(String* url);
    void        LoadJsonConfig();
    void        StoreJsonConfig(release_t* r);
    release_t   getLatestRelease();
    void        printRelease(release_t* r);
    
    String      json_url;
    stage_t     stage;
    release_t   currentRelease;
    bool        automode;
    bool        updateError;
    uint32_t    interval;
    bool        DoUpdate = false;
    uint32_t    lastupdate;

    std::vector<release_t>* releases = NULL;
};

#endif
