//-------------------------Headers-----------------------------------
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoJson.h>
#include <ESP8266WiFiMulti.h>
//----------------------------------------------------------------------

extern "C" {
  #include<user_interface.h>
}

//--------------------Globle Variable Declaration-----------------------

const char *ssid = "Charuka1";
const char *pwd = "123456789101";
ESP8266WiFiMulti WiFiMulti;
StaticJsonDocument<200> doc;
char packetBuffer[100];
int InitialReply = 0;
int ActivateBackup_AP = 0;
int SendInitial_Packet = 0;
int SendStatusTimer = 0;

//------------This ip address use for activating backup AP---------------

IPAddress local_IP(192,168,6,10);
IPAddress gateway(192,168,5,10); //4.9
IPAddress subnet(255,255,255,0);
IPAddress ServerIP(192,168,5,10); 
//-----------------------------------------------------------------------

unsigned int locPort1 = 1000; 
unsigned int locPort2 = 2000; 

WiFiUDP udp1;
WiFiUDP udp2;
//-----------------------------------------------------------------------

void setup() {
  
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');
  udp1.begin(locPort1);
  udp2.begin(locPort2);

    ConnectToAP();
    delay(1000);
}

//------------------------------------------Loop Function Start----------------------------------------------
void loop() {
  
  int cb=0;
  int sb =0 ;
  cb = udp1.parsePacket();   
  sb = udp2.parsePacket(); 
  
  SendStatusTimer++;
  
  if(WiFiMulti.run() != WL_CONNECTED){
      ConnectToAP();
    }

  if(WiFiMulti.run() == WL_CONNECTED && InitialReply == 1 ){
     Serial.println("send sensor data");
     SendGeoPhonedata();
     delay(1000);
    }
    
  if(cb > 0){
    
      udp1.read(packetBuffer,100);
      DeserializationError error = deserializeJson(doc, packetBuffer);
      int value = doc["ActivateBackup_AP"];
      ActivateBackup_AP=value;
      Serial.println(value);
      cb = 0;
      InitialReply = 1; //got the reply from the sever(AP)
    }
    
  if(SendInitial_Packet == 0  && WiFiMulti.run() == WL_CONNECTED){

     SendInitialPacket();
     SendInitial_Packet = 1;
     delay(1000); 
    }

  if(SendStatusTimer == 10){
      SendStatus();
    }
}

void ConnectToAP(){
           
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pwd);

  Serial.print("Distance = "); Serial.println(100);
  
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println("...");

    int i = 0;
    while (WiFiMulti.run() != WL_CONNECTED && i <20) { 
    delay(1000);
    Serial.print(++i); Serial.print(' ');
  }
    Serial.println('\n');
    Serial.println("Connection established!");  
    Serial.print("IP address:\t");
    Serial.println(WiFi.localIP());

}

void SendGeoPhonedata(){

  StaticJsonDocument<500> jsonBuffer;
  Serial.println(ServerIP);
  
  udp1.beginPacket(ServerIP, locPort1); //gateway
  JsonObject Sob1 = jsonBuffer.to<JsonObject>();
  Sob1["SensorData"] = 55500.55;
  Sob1["ID"] = 001;
  serializeJson(jsonBuffer, udp1);
    
  udp1.println();
  udp1.endPacket();
  
  Serial.println();
  Serial.println("Program is in SendGeoPhonedata function");
}

void SendInitialPacket(){

  StaticJsonDocument<500> jsonBuffer;
  
  udp2.beginPacket(ServerIP, locPort2);
  JsonObject ob1 = jsonBuffer.to<JsonObject>();
  ob1["Distance"] = 100;
  ob1["ID"] = 001;
  serializeJson(jsonBuffer, udp2);
    
   udp2.println();
   udp2.endPacket();
   Serial.println();
   Serial.println("Program is in SendInitialPacket function");
}

void SendStatus(){

  SendStatusTimer = 0;
  delay(1000);
  StaticJsonDocument<500> jsonBuffer;
  Serial.println("I'm here");
  
  udp2.beginPacket(ServerIP, locPort1);
  JsonObject ob1 = jsonBuffer.to<JsonObject>();
  ob1["Type"] = "Status";
  ob1["ID"] = 001;
  serializeJson(jsonBuffer, udp2);
    
   udp2.println();
   udp2.endPacket();
   
   Serial.println();
   Serial.println("Program is in SendStaus function");
}
