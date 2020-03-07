#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <FS.h>   //Include File System Headers

ESP8266WebServer server(80);

//---pin out---
int RedLed = 16;
int GreenLed=14;
int BlueLed=15;

//--- OTA/mDNS start data ---
const char OtaPassword[] = "Yourpassword";
const char OtaHostName[] = "YourhostName";
int OtaPort = 15727;

//--- wifi start data ---
const char HostName[] = "YourWifiHostNmae";
const char Password[] = "YouWifiPassword";

//--- htlm file ---
String Html;
  
void setup() {

  Serial.begin(115200);
  Serial.println("Start");
  

  StartWifi();
  //---mDNS isn´t necesary while using OTA---
  //Start_mDNS();
  LoadHtmlTemplate();
  StartLedServer();
  StartOTA();
}

void loop() {
  
  server.handleClient();
  ArduinoOTA.handle();         
}

void LoadHtmlTemplate(){
 
  if(SPIFFS.begin()){
      Serial.println("SPIDFFS started correctly");
      
      File htmlFile = SPIFFS.open("/index.html", "r");
      
      if(!htmlFile){
        Serial.println("Failed to open html file for reading");
        Html = "<h1>Error loading the file </h1>";
      }
      
      Html = htmlFile.readString();
      
      htmlFile.close();
      Serial.print("loaded: ");
      Serial.println(Html);

  }else{
    Serial.println("SPIDFFS error on start");
    Html = "<h1>Error at initiating the file system </h1>";
  }

  
  }

void HandleRoot() {  
  server.send(200, "text/html",  Html);
  }

void HandleLedData() {
  
  if(server.hasArg("red")){
    int value = server.arg("red").toInt();
    analogWrite(RedLed, value);
      Serial.println("red: "+String(value));
    }else{
      analogWrite(RedLed, 0);
       
     }

  if(server.hasArg("green")){
    int value = server.arg("green").toInt();
    analogWrite(GreenLed, value);
      Serial.println("green: "+String(value));
    }else{
      analogWrite(GreenLed, 0);
      
     }

  if(server.hasArg("blue")){
    int value = server.arg("blue").toInt();
    analogWrite(BlueLed, value);   
    Serial.println("blue: "+String(value));
    }else{
      
      analogWrite(BlueLed, 0);
     }
     
  }

void StartLedServer(){
  
  analogWrite(RedLed, 0);
  analogWrite(GreenLed, 0);
  analogWrite(BlueLed, 0);

  server.on("/", HandleRoot);
  server.on("/PostLedLata", HandleLedData);
  
  server.onNotFound(handleWebRequests);
  server.begin();

  Serial.println("Server Started");
}

void handleWebRequests(){

  Serial.print(loadFromSpiffs(server.uri()));
  
  if(loadFromSpiffs(server.uri())) return;
  String message = "File Not Detected\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " NAME:"+server.argName(i) + "\n VALUE:" + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
  Serial.println(message);
}


bool loadFromSpiffs(String path){
  String dataType = "text/plain";
  if(path.endsWith("/")) path += "index.htm";

  if(path.endsWith(".src")) path = path.substring(0, path.lastIndexOf("."));
  else if(path.endsWith(".html")) dataType = "text/html";
  else if(path.endsWith(".htm")) dataType = "text/html";
  else if(path.endsWith(".css")) dataType = "text/css";
  else if(path.endsWith(".js")) dataType = "application/javascript";
  else if(path.endsWith(".png")) dataType = "image/png";
  else if(path.endsWith(".gif")) dataType = "image/gif";
  else if(path.endsWith(".jpg")) dataType = "image/jpeg";
  else if(path.endsWith(".ico")) dataType = "image/x-icon";
  else if(path.endsWith(".xml")) dataType = "text/xml";
  else if(path.endsWith(".pdf")) dataType = "application/pdf";
  else if(path.endsWith(".zip")) dataType = "application/zip";
  File dataFile = SPIFFS.open(path.c_str(), "r");
  if (server.hasArg("download")) dataType = "application/octet-stream";
  if (server.streamFile(dataFile, dataType) != dataFile.size()) {
  }
  Serial.println("----"); 
  Serial.println(path);
  Serial.println("----");
  
  
  dataFile.close();
  return true;
}

void Start_mDNS(){
 if (!MDNS.begin(OtaHostName)) 
 {             
   Serial.println("Error starting mDNS");
 }
 Serial.println("mDNS started");
}

void StartWifi(){
  WiFi.begin(HostName, Password);

  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
    
  }
  Serial.println("conected");

  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void StartOTA(){
  SPIFFS.end();
  
  // Port defaults to 8266
  ArduinoOTA.setPort(OtaPort);

  // Hostname defaults to esp8266-[ChipID]
  ArduinoOTA.setHostname(OtaHostName);

  // No authentication by default
  ArduinoOTA.setPassword(OtaPassword);

  // Password can be set with it's md5 value as well
  //MD5(admin) = cfcd208495d565ef66e7dff9f98764da
  //ArduinoOTA.setPasswordHash("cfcd208495d565ef66e7dff9f98764da");
  
  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH)
      type = "sketch";
    else // U_SPIFFS
      type = "filesystem";

    // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
    Serial.println("Start updating " + type);
  });
  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });
  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
  });
  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
    else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
    else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
    else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
    else if (error == OTA_END_ERROR) Serial.println("End Failed");
  });
  ArduinoOTA.begin();
  Serial.println("OTA Ready");
}


  



