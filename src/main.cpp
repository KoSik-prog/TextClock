#include <Arduino.h>
#include <NTPClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <WiFiUdp.h>
#include <EEPROM.h>


/* PRIVATE VARIABLES */
//-------- WIFI ----------------
char *ssid[30];
char pass[30];
//-------- ACCESSPOINT ---------
#ifndef APSSID
#define APSSID "TextClock"
#define APPSK  "tclock"
#endif
const char *ap_ssid = "TextClock";
const char *ap_password = "tclock";
//------------------------------
uint8_t accessPointModeFlag = 0;
String password = "";
int ssidNumber = 0;
char buf[30];

/* FUNCTION PROTOTYPES */
void handleRoot();

/* FUNCTION DECLARATIONS */
void accesspoint_init(void){
  Serial.print("\nConfiguring access point...");
  WiFi.softAP(ap_ssid, ap_password);/* You can remove the password parameter if you want the AP to be open. */

  IPAddress myIP = WiFi.softAPIP();
  Serial.print("\nAP IP address: ");
  Serial.println(myIP);
  server.on("/", handleRoot);
  server.begin();
  Serial.println("HTTP server started");
}

void handleRoot() {
  String ssidList = "";

  int n = WiFi.scanNetworks();
  if (n == 0) {
    ssidList += "no networks found<br>";
  } else {
    for (int i = 0; i < n; ++i) {
      ssidList += String(i);
      ssidList += ": ";
      ssidList += (WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")");
      ssidList += ((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      ssidList += "<br>";
    }
  }
  
  server.send(200, "text/html", "<h1>Text Clock by KoSik</h1><br><br><br>" + ssidList);
}

void scan_networks(void){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("no networks found");
  } else {
    Serial.print(n);
    Serial.println(n + " networks found:");
    for (int i = 0; i < n; ++i) {
      Serial.print(i);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i) + " (" + WiFi.RSSI(i) + ")");
      Serial.println((WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*");
      delay(10);
    }
  }
}

int get_ssid(void){
  String inText = "";
  int ssidNr = 0;
  scan_networks();
  Serial.print("\nenter SSID: ");
  while(inText == ""){
      if(Serial.available()){
          inText = Serial.readStringUntil('\n');
          ssidNr = inText.toInt();
          Serial.println(WiFi.SSID(ssidNr));
      }
   }
   return ssidNr;
}

void get_wifi_password(void){
  Serial.print("enter password: ");
  while(password == ""){
      if(Serial.available()){
          password = Serial.readStringUntil('\n');
          Serial.println(password);
      }
   }
}

void save_password_to_eeprom(String epass, int eepromCellSize){
  for(uint8_t i=0; i<eepromCellSize;i++){
    EEPROM.write(0x00+i, epass[i]);
  }
  EEPROM.write(0x00+epass.length()+1, '\n');
  EEPROM.commit();
}

void get_password_from_eeprom(char *epass){
  uint8_t addr=0;
  Serial.print("from eeprom: ");
  for(uint8_t i=0; i<30; i++){
    if(EEPROM.read(addr) == '\n'){
      break;
    }
    epass[i] = char(EEPROM.read(addr));
    addr++;  
  }
}


/* MAIN */
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "europe.pool.ntp.org", 3600, 60000);
ESP8266WebServer server(80);


void setup(){
  EEPROM.begin(256);
  Serial.begin(115200);
  Serial.println("\n\n...:::  Text Clock v.1.0  :::...\n\n");

  //save_password_to_eeprom("password", sizeof(pass));
  get_password_from_eeprom(&pass[0]);
  Serial.print("PASS: ");
  Serial.println(pass);
  Serial.println(sizeof(pass));
  Serial.print("\n\n");

  
  ssidNumber = get_ssid();
  //get_wifi_password();

  //password.toCharArray(pass, password.length()+1);
  WiFi.begin(WiFi.SSID(ssidNumber), (const char*)pass);

  Serial.print("\nConnecting");
  uint16_t timer = 0;
  uint8_t connectedFlag = 1;
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print (".");
    timer++;
    if(timer > 16){
      accessPointModeFlag = 1;
      connectedFlag = 0;
      break;
    }
  }
  if(connectedFlag == 1){
    timeClient.begin();
    Serial.println
    ("\nConnected to WiFi");
  } else {
    accesspoint_init();
  }
}

void loop() {
  if(accessPointModeFlag == 1){
    server.handleClient();
  } else {
    timeClient.update();
    Serial.println(timeClient.getFormattedTime());
    delay(1000); 
  }
}