/*
*
* Copyright (C) 2018, Hugo van Santen
* All rights reserved.
*
* Please see the LICENSE file for more information.
*
*/

#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include "aes.h"
#include "PN532.h"
#include "udp_time.h"
#include <TimeLib.h>
#include "a116.h"

#include <stdlib.h>
#include <stdio.h>

constexpr uint8_t RST_PIN = 0;          // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 15;         // Configurable, see typical pin layout above
   

const char* ssid = "*****";
const char* password = "*****";


byte host[] = { 192,168,0,73 };
time_t time_from_last_opening=0;
//use 2 different keys to keep communication asymetrical

uchar encrypt_key[32]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                       0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
uchar decrypt_key[32]={0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,
                       0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff};
WiFiServer server(80);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance
typedef enum{
  door_open_half_closed,
  door_open_closed,
  door_half_closed,
  door_closed
}door_status;

uint8_t hourly;
bool connected=false;
door_status door;
udp_time udptime;
bool motor_running =false;
uint64_t compteur=0;

void decrypt(uchar* in,uchar* out);
void encrypt(uchar* in,uchar* out);
void hex2char(uchar* in, char* out, int size);
#define door_closed_position (0x0)
#define door_half_closed_position (0x1cf)
#define door_open_closed_position (0x3a0)  
#define door_open_half_closed_position (0x3a0)
#define opening_half_closed_time 3
#define opening_closed_time 10

//Reconnect wifi
void reconnect()
{
    connected=false;
    WiFi.disconnect();
    WiFi.mode(WIFI_OFF);
    delay(500);
    WiFi.mode(WIFI_STA);
    delay(500);
    WiFi.begin(ssid, password);
    Serial.println("reconnect");
}

//answer http request
void check_http(){
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
  // Wait until the client sends some data
  while(!client.available()){
    delay(1);
  }
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("door ok");
 
  client.println("</html>");
  delay(1);
  client.stop();
  delay(1);
}
//Quick way to program a char to hex function 
int hextoint(char c)
{
  switch (c)
  {
    case '0':
      return 0;
    case '1':
      return 1;
    case '2':
      return 2;
    case '3':
      return 3;
    case '4':
      return 4;
    case '5':
      return 5;
    case '6':
      return 6;
    case '7':
      return 7;
    case '8':
      return 8;
    case '9':
      return 9;
    case 'a':
      return 10;
    case 'b':
      return 11;
    case 'c':
      return 12;
    case 'd':
      return 13;
    case 'e':
      return 14;
    case 'f':  
      return 15;
    default:
      return 0;
  }
}

bool check_uid(char* key1,char* key2, char* key){
  WiFiClient client;
  const int httpPort = 80;
  if (!client.connect(host, httpPort)) {
    Serial.println("connection failed");
    reconnect(); 
    return false; 
  }
 connected=true;
 
 client.println("POST / HTTP/1.1");
 client.println("Host: server_name");
 client.println("Accept: */*");
 client.println("Content-Type: application/x-www-form-urlencoded");
 client.print("Content-Length: ");
 client.println(75);
 client.println();
 client.print("KEY1=");
 client.print(key1);
 client.print("&KEY2=");
 client.print(key2);
 
 delay(200); // Can be changed
 char c='n';
 String response="";
 while (client.available()) {
    c = client.read();
    response+=c;
  }
  response=response.substring(response.length()-32);

  if (client.connected()) { 
    client.stop();  // DISCONNECT FROM THE SERVER
  }
  char charResponse[33];
  uchar encrypted_key[16];
  response.toCharArray(charResponse,33);
  uint8_t i;
  for(i=0;i<16;i++)
    encrypted_key[i]=hextoint(charResponse[2*i])*16+hextoint(charResponse[2*i+1]);
  uchar decrypted_key[16];
  decrypt((uchar*)encrypted_key,decrypted_key);
  Serial.println("check");
  Serial.println((char*)charResponse);
  for(i=0;i<16;i++){
    Serial.print(encrypted_key[i]);
    Serial.print(" ");    
  }
  Serial.println(" ");    
  for(i=0;i<16;i++){
    Serial.print(decrypted_key[i]);
    Serial.print(" ");    
  }
  Serial.println(" ");    
  Serial.println(key);
  for(i=0;i<16;i++){
    if(decrypted_key[i]!=key[i])
      return false;
  }
  return true;
}

//generate a 7 byte random number from weak analog A1 bit.
void generate(char* chaine){
  uint8_t i,j;
  for(i=0;i<7;i++)
  {
    uint8_t temp=0;
    for(j=0;j<7;j++)
    {
      temp=temp*2+(analogRead(A0)&1);
      delayMicroseconds(100);
    }
    sprintf(&chaine[i*2],"%02x",temp);
  }
  
}

//Check nfc. return true and uid if card has correct state. else false if missing or wrong security
bool check_nfc(uint8_t* uid){
  // Look for new cards
  if(compteur%100==0)
  {
    mfrc522.PCD_Reset();
    mfrc522.PCD_Init();
    mfrc522.PCD_SetAntennaGain(7<<4); 
  }
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  Serial.println(now());
  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  uint8_t i;
  uid[0]=mfrc522.uid.size;
  for(i=0;i<mfrc522.uid.size;i++)
    uid[i+1]=mfrc522.uid.uidByte[i];

  byte PSWBuff[] = {0xff, 0xff, 0xff, 0xff}; //32 bit PassWord default FFFFFFFF
  byte pACK[] = {0x0, 0x0}; //16 bit PassWord ACK returned by the NFCtag
  mfrc522.PCD_NTAG216_AUTH(&PSWBuff[0], pACK);
  if (pACK[0]== 0xff && pACK[1]== 0xff)
  {
    return true;
  }
  if (pACK[0]== 0xff && pACK[1]== 0xff && door!=door_closed)
  {
    return true;
  }
    delay(1000);
    return false;
}

//EAS-256bit encrypt and decrypt functions
void encrypt(uchar* in,uchar* out){
   long unsigned int key_schedule[60];
   KeyExpansion(encrypt_key,key_schedule,256);
   aes_encrypt_1(in,out,key_schedule,256);   
}
void decrypt(uchar* in,uchar* out){
   long unsigned int key_schedule[60];
   KeyExpansion(decrypt_key,key_schedule,256);
   aes_decrypt_1(in,out,key_schedule,256);   
}

//Serial print char table. Useful for debug.
void print_char_tab(char* tab,int len){
  uint8_t i;
  for(i = 0; i < len; i++)
   {
  Serial.print(tab[i]);
  }
  Serial.println("");
}

//Serial print a hex table. Useful for debug.
void print_hex_tab(char* tab,int len){
  uint8_t i;
  for(i = 0; i < len; i++)
  {
    char temp[3];
    sprintf(temp,"%02x",tab[i]);
    Serial.print(temp);
  }
  Serial.println("");
}

//Change a int chain from 0x0 to 0xf, into character string
void hex2char(uchar* in, char* out, int size){
  uint8_t i;
  for(i=0;i<size;i++)
    sprintf(&out[2*i],"%02x",in[i]);
  out[2*size]='\0';
}

time_t getNtpTime(){
  return udptime.udp_get_time();
}

    

//Check servo
void check_servo(){
  uint16_t position ;
  position = a116::get_position(1);
//Color configuration is not working yet
//  if(connected ==true)
//    a116::set_color(1,true,false,false,false);
//  else
//    a116::set_color(1,false,true,false,false);
//TODO check if servo is in protection mode
 Serial.print("servo state");
 Serial.println(a116::get_state(1));
  if((a116::get_state(1)&0b00001111)!=0)
  {
    a116::servo_reboot(1);
    delay(5000);
  }


  switch(door){

    case door_open_half_closed:
      if(position!=door_open_half_closed_position)
      {
        a116::servo_move(1,door_open_half_closed_position);
        motor_running=true;
      }
      else
        motor_running=false; 
      break;
    case door_open_closed:
      if(position!=door_open_closed_position)
      {
        a116::servo_move(1,door_open_closed_position);
        motor_running=true;
      }
      else
        motor_running=false;        
      break;
    case door_half_closed:
      if(position!=door_half_closed_position)
      {
        a116::servo_move(1,door_half_closed_position);
        motor_running=true;
      }
      else
        motor_running=false;        
      break;
    case door_closed:
      if(position!=door_closed_position)
      {
        a116::servo_move(1,door_closed_position);
        motor_running=true;
      }
      else
        motor_running=false;        
      break;
  }
    
}

//To check switch, it will check continuity by a square signal from pin 4 to 5, to be sure the button is really pressed.
bool  check_switch(){
  digitalWrite(4,LOW);
  if(digitalRead(5)==LOW)
  {
    uint8_t i;
    for(i = 1;i<10;i++)
    {
      digitalWrite(4,i&0b1);
      delay(1);
      if(digitalRead(5)!=digitalRead(4))
        return false;
    }
    return true;
  }
  return false;
}

void setup() {
  Serial.begin(115200);  
  delay(1000);


    
  pinMode(5, INPUT_PULLUP);
  pinMode(4, OUTPUT);

  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  mfrc522.PCD_SetAntennaGain(7<<4); 
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));
  mfrc522.PICC_DumpToSerial(&(mfrc522.uid));

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
 
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  connected=true;


  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
  door = door_closed;
  udptime.upd_time_setup();
  setSyncProvider(getNtpTime);
  setSyncInterval((time_t)86400); 
  ESP.wdtEnable(1000);

}

void loop() {
  uint8_t uid[8];
  bool success;
  //Check WIFI
  if(WiFi.isConnected()!=true)
    Serial.println("disconnected");
  if(WiFi.isConnected()!= true && connected!=false)
  {
    Serial.println("disconnected");
    reconnect();
  }
  if(compteur%1000==0)
  {
    WiFiClient client;
    if (!client.connect(host, 80))
      reconnect(); 
    else
    {
      Serial.println("connected!");
      connected=true;
      client.stop();
    }
  }
  
  //Check Web request
  check_http();

  //Check if it must make an automatic door change
  if((door == door_open_half_closed) && (now()-time_from_last_opening>opening_half_closed_time))
  {
    door = door_half_closed;
    Serial.println("on ferme à moitié");
  }
  if(door == door_open_closed && now()-time_from_last_opening>opening_closed_time)
  {
    door = door_closed;
    Serial.println("on ferme");
  }
  if(door == door_half_closed && hour()>19)
  {
    door = door_closed;
    Serial.println("on ferme");
  }

  //Check if button is pressed
  if(check_switch()== true)
  {
    door = door_open_closed;
    time_from_last_opening = now();
  }

  //Check state and position of servo
  check_servo();
  
  //Check NFC card  
  success = check_nfc(uid);
  if(success == true)
  {
    Serial.println("RFID!");
    char key[2][16];
    uchar encrypted_key[2][16];
    char charencrypted_key[2][33];
    sprintf(key[0],"%02x%02x%02x%02x%02x%02x%02x%02x",
      uid[0],uid[1],uid[2],uid[3],uid[4],uid[5],uid[6],uid[7]);
    generate(key[1]);
    sprintf(&key[1][14],"%02x",1);
    encrypt((unsigned char*)key[0],encrypted_key[0]);
    encrypt((unsigned char*)key[1],encrypted_key[1]);
    hex2char(encrypted_key[0],charencrypted_key[0],16);
    hex2char(encrypted_key[1],charencrypted_key[1],16);
    Serial.println(charencrypted_key[0]);
    Serial.println(charencrypted_key[1]);
    bool door_open;
    door_open = check_uid(charencrypted_key[0],charencrypted_key[1], key[1]);
    if(door_open == true){
      Serial.println("on ouvre!");
      door=door_open_half_closed;
      time_from_last_opening = now();
    }
    else
      Serial.println("au revoir!");
  }
  compteur++;
  ESP.wdtFeed();

}
 
