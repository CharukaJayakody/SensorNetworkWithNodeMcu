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

#define LED1 D1
#define LED2 D6

const char *ssid ="Charuka"; 
const char *pwd = "12345678910";
ESP8266WiFiMulti WiFiMulti;

IPAddress local_IP(192,168,5,10); 
IPAddress gateway(192,168,4,9); 
IPAddress subnet(255,255,255,0);
IPAddress NextAP(192,168,4,9);
StaticJsonDocument<200> doc;
unsigned int locPort1 = 1000;
unsigned int locPort2 = 2000;
WiFiUDP udp1; 
WiFiUDP udp2; 
char packetBuffer[100];
IPAddress addrs;

IPAddress IpAddressArr[10];
int DistanceArr[]={0};
int IpCount=0;
int DCount=0;
int OnArrTimer = 0; 
int ConnectedClientCount = 0;
int InitialPacketSend = 0;

void setup() {
  
  Serial.begin(9600);
  delay(10);
  Serial.println('\n');

  ConnectToAP();
  delay(1000);
  EnableAP();
  delay(1000);
  
  udp1.begin(locPort1);
  udp2.begin(locPort2);
}

void loop() {


int Icb = 0;
int Scb = 0;
int Acb = 0;
  
  Scb = udp1.parsePacket();
  Icb = udp2.parsePacket();// listen for initial data packet

  ConnectedClientCount = wifi_softap_get_station_num();
  
  if(Scb > 0){
      GetSensorDataPacket();
      Scb = 0;
    }
    
  if(ConnectedClientCount > 0){
      if(InitialPacketSend == 0){
        OnArrTimer++;
        delay(1000);
      }         
    }
    
   if(Icb > 0){
      delay(500);
      GetUdpDataPacket();
      Icb = 0;
    }

   if(OnArrTimer == 25){
       SendReply_NxtAP();
    }
//    delay(1000);
//    SendDataToNextAP(500,001);
}

void EnableAP(){

  WiFi.softAPConfig(local_IP, gateway, subnet);
  WiFi.softAP("Charuka1", "123456789101");// channel = 9 
  delay(1000);
              
  Serial.print("Access Point \"");
  Serial.print("Charuka1");
  Serial.println("\" started");
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP()); 
}

void ConnectToAP(){
         
  WiFi.mode(WIFI_STA);
  WiFiMulti.addAP(ssid, pwd);

  Serial.print("Connecting to ...........");
  Serial.print(ssid); Serial.println("...");

  int i = 0;
  while (WiFiMulti.run() != WL_CONNECTED) {    
   
   delay(1000);
   Serial.print(++i); Serial.print(' ');
  }
  
  Serial.println('\n');
  Serial.println("Connection established!");  
  Serial.print("IP address:\t");
  Serial.println(WiFi.localIP());
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
  
  SendDataToNextAP(SensorData,ClientID); 
//    udp2.beginPacket(NextAP, locPort1);
//    udp2.write("Hi");
//    udp2.endPacket();
    Serial.println("Sent a Udp packet");
}

void SendDataToNextAP(double Sensor_Data,int Client_ID){
  
  StaticJsonDocument<500> jsonBuffr;
  Serial.println("I'm in SendDataToNextAP Function");
  
  udp2.beginPacket(NextAP, locPort1); //gateway
  JsonObject Sob1 = jsonBuffr.to<JsonObject>();
  Sob1["SensorData"] = Sensor_Data;
  Sob1["ID"] = Client_ID;
  serializeJson(jsonBuffr, udp2);
    
//    udp2.write("Hi");
//    udp2.endPacket();
  
  udp2.println();
  udp2.endPacket();
  Serial.println("Sent a Udp packet"); 
  delay(1000);
}

void GetUdpDataPacket(){
  
  digitalWrite(LED2, HIGH);
  udp2.read(packetBuffer,100);
  DeserializationError error = deserializeJson(doc, packetBuffer);
  
  addrs = udp2.remoteIP(); // Identify ip address of the recived udp packet
  int Distance = doc["Distance"];
  int ClientID = doc["ID"];
    
  IpAddressArr[IpCount]=addrs;
  IpCount++;
  DistanceArr[DCount]=Distance;
  DCount++;
  
  Serial.println(); 
  Serial.print("Distance To Central Point = ");
  Serial.println(Distance);
  Serial.print("Client ID = ");
  Serial.println(ClientID);

  Serial.println(); 
  Serial.print("IP adress is = ");
  Serial.print((addrs));
  Serial.println("");
}

void SendReply_NxtAP(){
  
    Serial.println("SendReply_NxtAP");
    
    Serial.print("Dcount = "); Serial.println(DCount);

   Serial.println(DistanceArr[0]);
   Serial.println(DistanceArr[1]);
   Serial.println(DistanceArr[2]);
    
    int j=0;
    int minDistance = 0;
    int minDistanceIndex = 0;
    minDistance = DistanceArr[0];
    
    for (j = 0; j < DCount ; j++)
    {
        if (minDistance > DistanceArr[j]){
          
            minDistance = DistanceArr[j];
            minDistanceIndex = j;
            Serial.print("minDistanceIndex ");Serial.println(j);
        }
    }
    Serial.print("Minimum Distance is ");Serial.println(minDistance);

    for (int x = 0; x < DCount ; x++)
    { 
        Serial.print("X = "); Serial.println(x);
        if(x == minDistanceIndex){
         
         StaticJsonDocument<500> jsonBuffer;
         udp1.beginPacket(IpAddressArr[minDistanceIndex], locPort1);
         JsonObject ob1 = jsonBuffer.to<JsonObject>();
         ob1["ActivateBackup_AP"] = 1;
         serializeJson(jsonBuffer, udp1); 
          
         udp1.println();
         udp1.endPacket(); 

         delay(1000);
        }
        
        else{
          
         StaticJsonDocument<500> jsonBuffer;
         udp1.beginPacket(IpAddressArr[x], locPort1);
         JsonObject ob1 = jsonBuffer.to<JsonObject>();
         ob1["ActivateBackup_AP"] = 0;
         serializeJson(jsonBuffer, udp1);

         udp1.println();
         udp1.endPacket();

         delay(1000);
        }
    }    
    OnArrTimer = 26;
    InitialPacketSend =  1; // 
}
