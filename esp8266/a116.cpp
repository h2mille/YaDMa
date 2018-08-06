/*
*
* Copyright (C) 2018, Hugo van Santen
* All rights reserved.
*
* Please see the LICENSE file for more information.
*
*/
#include "a116.h"
#include <stdlib.h>
#include <stdio.h>

void a116::servo_write(uint8_t id,uint8_t  cmd,uint8_t packet_size, uint8_t* data){
  uint8_t i;
  uint8_t send_packet[255];
  send_packet[0]=0xff;
  send_packet[1]=0xff;
  send_packet[2]=packet_size+7;
  send_packet[3]=id;
  send_packet[4]=cmd;
  for(i=7;i<send_packet[2];i++)
    send_packet[i]=data[i-7];

  //set checksum
  send_packet[5]=send_packet[2]^send_packet[3]^send_packet[4];
  for(uint8_t i=0;i<packet_size;i++)
    send_packet[5]=send_packet[5]^send_packet[i+7];
  send_packet[5]=send_packet[5]&0xfe;
  send_packet[6]=(~send_packet[5])&0xfe;
  delay(100);
  for(i=0;i<send_packet[2];i++)
  {
    Serial.print((char)send_packet[i]);
  }
}

void a116::servo_read_ram(uint8_t id, uint8_t address,uint8_t size,uint8_t* output)
{
  delay(10);
  while(Serial.available())
    Serial.read();
  uint8_t i;
  uint8_t send_packet[255];
  uint8_t receive_packet[255];
  send_packet[0]=0xff;
  send_packet[1]=0xff;
  send_packet[2]=9;
  send_packet[3]=id;
  send_packet[4]=0x4;
  send_packet[7]=address;
  send_packet[8]=size;
  //set checksum
  send_packet[5]=send_packet[2]^send_packet[3]^send_packet[4]^send_packet[7]^send_packet[8];
  send_packet[5]=send_packet[5]&0xfe;
  send_packet[6]=(~send_packet[5])&0xfe;
  Serial.println();
  for(i=0;i<send_packet[2];i++)
  {
    Serial.print((char)send_packet[i]);
  }
  delay(10);
  i=0;
  while(Serial.available())
  {
    receive_packet[i]= Serial.read();
    i++;
  }
  memcpy(output,&receive_packet[11],size);
}

void a116::set_color(uint8_t id, bool white, bool blue, bool green, bool red)
{
  uint8_t packet[3];
  uint8_t start_address=53;
  uint8_t lengh=1;
  uint8_t value=(red<<3)+(green<<2)+(blue<<1)+white;
  packet[0]=start_address;
  packet[1]=lengh;
  packet[2]=value;
  servo_write(id,3,3, packet);
}

void a116::servo_reboot(uint8_t id)
{
  servo_write(id,9,0, NULL);
}

uint16_t a116::get_position(uint8_t id){
  uint8_t output[2];
  servo_read_ram(id,60,2,output);
  return (uint16_t)output[0]+16*(uint16_t)output[1];
}
uint8_t a116::get_state(uint8_t id){
  uint8_t output[1];
  servo_read_ram(id,48,1,output);
  Serial.print("servo status");
  Serial.print(output[0]);
  return (uint16_t)output[0];
}

void a116::servo_move(uint8_t id,uint16_t angle)
{
  uint8_t packet[5];
  packet[0]=angle&0xff;
  packet[1]=(angle>>8)&0xff;
  packet[2]=0;
  packet[3]=1;
  packet[4]=0;
  servo_write(id,5,5, packet);
}
