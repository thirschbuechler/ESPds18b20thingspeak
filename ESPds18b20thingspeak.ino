#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>//no hard-coded wifi credentials
#include "sensorconfig.h"
//provides *adresses, *adresses2, key, server
//        ONE_WIRE_BUS, maxdisc, myPeriodic, thermos


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);


int sent = 0;
void setup() {
  Serial.begin(115200);
  Serial.println("serial set-up");
  //Serial.print("%c",*my.me[0]);
  connectWifi();
}

void(* resetFunc) (void) = 0;//declare reset function at address 0

// function to print a device address
void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    // zero pad the address if necessary
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.print("\n");
}

float gettempx(int numb) { //////////////////////////////////////////////////////////////////
  float temp = -127;
  temp = sensors.getTempC((unsigned char *) adresses[numb]);
  Serial.print(String(temp) + "°C ");
  if (temp > 125)
  {
    Serial.println("hi temp error: above 125°C");
    return (-127);
  }
  else if ((temp == -127) || (temp == 85)) {
    Serial.println("no sensor connected or just initialized");
    return (-127);
  }
  else if (temp < -55)
  {
    Serial.println("lo temp error: below -55°C");
    return (-127);
  }
  else
  {
    //String tempC = dtostrf(temp, 4, 1, buffer);//handled in sendTemp()
    Serial.println("loop No." + String(sent) + " Temperature");
    //Serial.println(temp);
    return (temp);

  }//end if/else
}//end gettempx

void loop() {//////////////////////////////////////////////////////////////////
  float temp;
  //char buffer[10];
  sensors.begin();//not needed until multiples??//hotswap works
  //delay(5000);
  //#ifdef __PARASITIC//this does not work, oh well, whatever
  //OneWire.write(0x44, 1);        // start conversion, with parasite power on at the end
  //Serial.println("supposedly wrote parasitic");
  //#endif
  sensors.requestTemperatures(); //query all devices
  Serial.println("query sent!");
  
  DeviceAddress currentaddress = {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  int i = 0;
  for (i = 0; i < (maxdisc); i++) {

    //Serial.println(sensors.getTempCByIndex(i));// temp i if no device for i+1
    if (!sensors.getAddress(currentaddress, i)) {

      //Serial.println("no Device" +String(i)); //Serial.println("Unable to find address for Device %d", i);//this breaks my heart
    }
    else {
      Serial.println("device " + String(i + 1) + " found");
      Serial.println(sensors.getTempCByIndex(i));
      printAddress(currentaddress);
    }
  }

  Serial.println("\naddress discovery over");

  sendTeperatureTS();

  int count = myPeriodic;
  while (count--)
    delay(1000);
}//end loop

void connectWifi()//////////////////////////////////////////////////////////////////
{
  Serial.print("Connecting using wifiManager");
  WiFiManager wifiManager;
  wifiManager.autoConnect("AutoConnectAP");//opens if previous conf. wifi is unreachable

  Serial.println("");
  Serial.println("Connected");
  Serial.println("");
}//end connect

void sendTeperatureTS()//////////////////////////////////////////////////////////////////
{
  WiFiClient client;

  if (client.connect(server, 80)) {
    Serial.println("WiFi Client connected ");

    String postStr = "";
    String apiKey = (char *)key;
    //Serial.println(apiKey);

    Serial.println("sensor request");
    sensors.requestTemperatures(); //query all devices
    Serial.println("sensor request over");

    int i = 1;
    float temp = -127;
    for (i = 1; i < (thermos + 1); i++) {

      temp = gettempx(i - 1);
      if (temp != -127)
      {
        postStr += "&field";
        postStr += String(i);
        postStr += "=";
        postStr += String(temp);
      }

    }

    postStr += "\r\n\r\n";

    client.print("POST /update HTTP/1.1\n");
    client.print("Host: api.thingspeak.com\n");
    client.print("Connection: close\n");
    client.print("X-THINGSPEAKAPIKEY: " + apiKey + "\n");
    client.print("Content-Type: application/x-www-form-urlencoded\n");
    client.print("Content-Length: ");
    client.print(postStr.length());
    client.print("\n\n");
    client.print(postStr);
    Serial.println("postStr:" + postStr);
    delay(1000);

  }//end if
  sent++;
  if (sent > 1000) {
    resetFunc();
  }
  client.stop();
}//end send
