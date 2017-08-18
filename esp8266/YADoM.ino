#include <ESP8266WiFi.h>
#include <SPI.h>
#include <MFRC522.h>
#include "aes.h"
#include <stdlib.h>
#include <stdio.h>

constexpr uint8_t RST_PIN = 0;          // Configurable, see typical pin layout above
constexpr uint8_t SS_PIN = 15;         // Configurable, see typical pin layout above
#include "aes.h"
#include "PN532.h"

const char* ssid = "cheznous";
const char* password = "annesophieethugo";
const char* host = "192.168.0.30";
//use 2 different keys to keep communication asymetrical
uchar encrypt_key[32]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                 0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11, 
                 0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,
                 0x1b,0x1c,0x1d,0x1e,0x1f};
uchar decrypt_key[32]={0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,
                 0x09,0x0a,0x0b,0x0c,0x0d,0x0e,0x0f,0x10,0x11, 
                 0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1a,
                 0x1b,0x1c,0x1d,0x1e,0x00};
WiFiServer server(80);
MFRC522 mfrc522(SS_PIN, RST_PIN);  // Create MFRC522 instance

void decrypt(uchar* in,uchar* out);
void encrypt(uchar* in,uchar* out);
void hex2char(uchar* in, char* out, int size);

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
}
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
    return false;
  }
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
   bool exit = false;
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
  for(i=0;i<16;i++){
    if(decrypted_key[i]!=key[i])
      return false;
  }
  return true;
}

void generate(char* chaine){
  uint8_t i,j;
  for(i=0;i<7;i++)
  {
    uint8_t temp=0;
    for(j=0;j<8;j++)
      temp=temp*2+(analogRead(A0)&1);
    sprintf(&chaine[i*2],"%02x",temp);
  }
  
}

bool check_nfc(uint8_t* uid){
  // Look for new cards
  if ( ! mfrc522.PICC_IsNewCardPresent()) {
    return false;
  }

  // Select one of the cards
  if ( ! mfrc522.PICC_ReadCardSerial()) {
    return false;
  }

  // Dump debug info about the card; PICC_HaltA() is automatically called
  uint8_t i;
  uid[0]=mfrc522.uid.size;
  for(i=0;i<mfrc522.uid.size;i++)
    uid[i+1]=mfrc522.uid.uidByte[i];
  
  return true;
}

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
void print_char_tab(char* tab,int len){
  uint8_t i;
  for(i = 0; i < len; i++)
   {
  Serial.print(tab[i]);
  }
  Serial.println("");
}
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

void setup() {
  Serial.begin(74880);  
  delay(1000);
  char temp[16];
  generate(temp);


  SPI.begin();      // Init SPI bus
  mfrc522.PCD_Init();   // Init MFRC522
  mfrc522.PCD_DumpVersionToSerial();  // Show details of PCD - MFRC522 Card Reader details
  Serial.println(F("Scan PICC to see UID, SAK, type, and data blocks..."));

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
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
 
}
void hex2char(uchar* in, char* out, int size){
  uint8_t i;
  for(i=0;i<size;i++)
    sprintf(&out[2*i],"%02x",in[i]);
  out[2*size]='\0';
}

void loop() {
  check_http();
  uint8_t uid[8];
  bool success;
  success = check_nfc(uid);
  if(success== true)
  {
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
    
    bool door_open;
    door_open = check_uid(charencrypted_key[0],charencrypted_key[1], key[1]);
    if(door_open == true)
      Serial.println("on ouvre!");
    else
      Serial.println("au revoir!");
   }
}
 
