
#ifndef UDP_TIME_H
#define UDP_TIME_H
#include <stdio.h>
#include <ESP8266WiFi.h>
#include <WiFiUdp.h>
#include <Time.h>

class udp_time{
  public:
    udp_time();
    void upd_time_setup();
    time_t udp_get_time();
    unsigned long sendNTPpacket(IPAddress& address);
  private:

    WiFiUDP udp;
    
};


#endif




