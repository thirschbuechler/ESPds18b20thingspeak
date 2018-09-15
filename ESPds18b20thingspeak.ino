#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>//no hard-coded wifi credentials
#include "sensorconfig.h"
//node-config: provides *adresses, *adresses2, key, server
//        ONE_WIRE_BUS, maxdisc, myPeriodic, thermos


OneWire oneWire(ONE_WIRE_BUS);

DallasTemperature sensors(&oneWire);

// global variables / counters
int loops = 0;
int uploadenabled=1; // turn off for temporary setup of new sensor without upload, if desired


// https://github.com/tzapu/WiFiManager
void connectWifi(){
  Serial.print("Connecting using wifiManager");
  WiFiManager wifiManager;
  wifiManager.setConfigPortalTimeout(360); // reboot after 360s if no new wifi entered
  wifiManager.autoConnect("AutoConnectAP"); // opens if previous conf. wifi is unreachable

  // continues only if connected
  Serial.println("Connected");
}//end connect

// setup serial and wifi
void setup() {
  Serial.begin(115200);
  Serial.println("serial set-up");
  connectWifi();
}

void(* resetFunc) (void) = 0;//declare reset function at address 0

// function to print a device address
void printAddress(DeviceAddress deviceAddress){
  
  for (uint8_t i = 0; i < 8; i++){
    if (deviceAddress[i] < 16) Serial.print("0"); // zero pad the address if necessary
    Serial.print(deviceAddress[i], HEX);
  }
  Serial.print("\n");
}

// get temperature of sensor $numb as specified in adresses in sensorconfig.h
float gettempx(int numb) {
  float temp = -127;
  temp = sensors.getTempC((unsigned char *) adresses[numb]);
  Serial.print(String(temp) + "°C ");
  
  if (temp > 125){
    Serial.println("hi temp error: above 125°C");
    return (-127);
  }
  else if ((temp == -127) || (temp == 85)) {
    Serial.println("no sensor connected or just initialized");
    return (-127);
  }
  else if (temp < -55){
    Serial.println("lo temp error: below -55°C");
    return (-127);
  }
  else{
    //String tempC = dtostrf(temp, 4, 1, buffer);//handled in sendTemp()
    Serial.println("loop No." + String(loops) + " Temperature");
    //Serial.println(temp);
    return (temp);

  }//end if/else
}//end gettempx


//connect to thingspeak, query sensors, upload data
void sendTeperatureTS(){
  WiFiClient client; //client object to connect to server

  if (client.connect(server, 80)) {
    Serial.println("connected to wifi and server");

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
      if (temp != -127){
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

  }//end if client.connect

  
  client.stop();
}//end send

///// main loop
void loop() {
  float temp;
  sensors.begin();//not needed until multiples??//hotswap works
  //#ifdef __PARASITIC//this does not work, oh well, whatever
  //OneWire.write(0x44, 1);        // start conversion, with parasite power on at the end
  //#endif
  
  sensors.requestTemperatures(); //query all devices
  Serial.println("query sent!");
  
  DeviceAddress currentaddress = {0x28, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
  int i = 0;
  
  for (i = 0; i < (maxdisc); i++) {

    //Serial.println(sensors.getTempCByIndex(i));// temp i if no device for i+1
    if (!sensors.getAddress(currentaddress, i)) {

      //Serial.println("no Device" +String(i));
    }
    else {
      Serial.println("device " + String(i + 1) + " found");
      Serial.println(sensors.getTempCByIndex(i));
      printAddress(currentaddress);
    }
  }

  Serial.println("\naddress discovery over");

  loops++;
  if (loops > 1000) {
    resetFunc();
  }

  
  if (uploadenabled){
    sendTeperatureTS();  
  }
  else{
    Serial.println("Upload disabled for setup readout");
  }
  
  int count = myPeriodic;
  while (count--) //api delay f. thingspeak
    delay(1000);
    
}//end loop


