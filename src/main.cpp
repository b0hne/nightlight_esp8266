
/********************
get time, activate light at dusk, deactivate at dawn
 ********************/
#include <ESP8266WiFi.h>
#include <TimeLib.h>
#include <WiFiUdp.h>

#include "sunset.h"
// München
#define LATITUDE 48.134978602501974
#define LONGITUDE 11.581345443987907

#define TIMEZONE 0
char ssid[] = "***";
char pass[] = "***";

const unsigned int localPort = 2390;
IPAddress timeServerIP;
const char* ntpServerName = "0.europe.pool.ntp.org";
const int buffersize = 48;
byte packetBuffer[buffersize];
WiFiUDP udp;
SunSet sun;
int dusk;
int dawn;
int counter ;
int light;

void getNTPdata(IPAddress& address)
{

  memset(packetBuffer, 0, buffersize);
  packetBuffer[0] = 0b11100011;
  packetBuffer[1] = 0;
  packetBuffer[2] = 6;
  packetBuffer[3] = 0xEC;
  packetBuffer[12] = 49;
  packetBuffer[13] = 0x4E;
  packetBuffer[14] = 49;
  packetBuffer[15] = 52;

  udp.beginPacket(address, 123);
  udp.write(packetBuffer, buffersize);
  udp.endPacket()
}

void setup()
{
  pinMode(5, OUTPUT);
  digitalWrite(5, HIGH);
  light = HIGH;

  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED)
  {
    delay(500);
  }
  udp.begin(localPort);
  sun.setPosition(LATITUDE, LONGITUDE, TIMEZONE);
}

void loop()
{
  if (counter % 600 == 0)
  {
    WiFi.hostByName(ntpServerName, timeServerIP);
    getNTPdata(timeServerIP);
  }
  if (counter % 10 - 1 == 0)
  {
    if (udp.parsePacket())
    {
      udp.read(packetBuffer, buffersize);
      unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);
      unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);
      time_t secsSince1900 = highWord << 16 | lowWord;
      time_t utcCalc = secsSince1900 - 2208988800UL;
      setTime(utcCalc);
    }
  }

  if (year() != 1970)
  {
    light = HIGH;
    sun.setCurrentDate(year(), month(), day());
    dusk = sun.calcSunset();
    int dusk_hour = (dusk / 60);
     if (dusk_hour <= hour())
    {
      int dusk_minute = (dusk % 60);
      if (!(dusk_hour == hour() && dusk_minute > minute()))
      {
        light = LOW;
      }
    }
    else
    {
      dawn = sun.calcSunrise();
      int dawn_hour = dawn / 60;
      if (dawn_hour >= hour())
      {
        int dawn_min = dawn % 60;
        if (!(dawn_hour == hour() && dawn_min < minute()))
        {
          light = LOW;
        }
      }
    }
    digitalWrite(5, light);
  }
  counter++;
  delay(1000);
}
