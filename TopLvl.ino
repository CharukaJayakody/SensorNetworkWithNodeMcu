//----------------------------Headers-----------------------------------
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

//#define LED1 D1
//#define LED2 D6

const char *ssid = "Charuka";
const char *pwd = "12345678910";
ESP8266WiFiMulti WiFiMulti;

IPAddress local_IP(192,168,4,9); // Ip address of this node
IPAddress gateway(192,168,4,8); 
IPAddress subnet(255,255,255,0);


StaticJsonDocument<200> doc;
unsigned int locPort1 = 1000;
WiFiUDP udp1;
char packetBuffer[100];

void setup() {
  
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');

  EnableAP(); 
  udp1.begin(locPort1);
  delay(1000);
}

void loop() {

int Icb = 0;
int Scb = 0;
  
  Icb = udp1.parsePacket();  
  if(Icb > 0){
      Serial.println("Got a Udp packet");
      GetSensorDataPacket();
    }
}

void EnableAP(){

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP(ssid,pwd);
  delay(1000);
              
  Serial.print("Access Point \"");
  Serial.print(ssid);
  Serial.println("\" started");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP()); 
}

void GetSensorDataPacket(){

  Serial.println("GetSensorDataPacket Function");
  udp1.read(packetBuffer,100);
  DeserializationError error = deserializeJson(doc, packetBuffer);

  IPAddress ClientAddrs = udp1.remoteIP();

  double SensorData = doc["SensorData"];
  int ClientID = doc["ID"];

  delay(1000);
  Serial.println(); 
  Serial.print("Sensor reading = ");
  Serial.println(SensorData);
  Serial.print("Client ID = ");
  Serial.println(ClientID);    
}
