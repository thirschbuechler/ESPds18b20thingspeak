#include <ESP8266WiFi.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include <WiFiManager.h>//no hard-coded wifi credentials
#include <math.h>  // for "isnan"
#include "sensorconfig.h"
//node-config: provides *adresses, *adresses2, key, server
//        ONE_WIRE_BUS, maxdisc, myPeriodic, thermos


#define sw_version "1.2" 

//// Changelog ////
// 1.01: renamed sendTeperature to sendTemperature + its 1x call
// 1.1: added isnan in buildstr
//       turned down maxloops to 500 (from 1000) after making it a var
//       added 0.0==total to buildstr reset condition (is very unlikely)
//       improved reset by using proper reset instruction instead of "0"-address
// 1.2: added delay(0) wifi stack breather to resolve watchdog reboots before uplink complete
//		https://www.esp8266.com/viewtopic.php?f=6&t=18819

// unused debug options:
// ToDo: try-catch? https://discourse.processing.org/t/try-catch-with-serial/1148/6
// ToDo: check whether resetfunc fails sometimes (worked in lab conditions), e.g. write values -10 to channels

int maxloops = 500;

//EspClass ESP;//gets created somewhere else and would be redeclaration (one of the wifi libs?)

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

//void(* resetFunc) (void) = 0;//declare reset function at address 0
void resetFunc(void){
  ESP.restart(); //clean restart with clearing registers instead of lazy reset with dirty registers (=address0)
  //one issue: first reboot has to be hardware, based, or it hangs with "ets Jan  8 2013,rst cause:2, boot mode:(1,7)"
  // see https://github.com/esp8266/Arduino/issues/1722
}

// function to get a device address as String
String strAddress(DeviceAddress deviceAddress){
  String ret="";
  for (uint8_t i = 0; i < 8; i++){
    delay(0); // wifi stack breather
    if (deviceAddress[i] < 16) ret+="0"; // zero pad the address if necessary
    ret+=String(deviceAddress[i], HEX);
  }
  ret+=("\n");
  return(ret);
}

//get name if available, this doesn't work for some reason
String nameforaddress(DeviceAddress deviceAddress){
  for (uint8_t i = 1; i < (thermos + 1); i++) {
    delay(0); // wifi stack breather
    if ((unsigned char *)addresses[i-1]==deviceAddress){
        return(String(names[i-1])+" "+strAddress(deviceAddress));
    }
    else{
        return("unknown: "+strAddress(deviceAddress));
    }
  }
  
}

// get temperature of sensor $numb as specified in adresses in sensorconfig.h
float gettempx(int numb) {
  float temp = -127;
  delay(0); // wifi stack breather
  temp = sensors.getTempC((unsigned char *) addresses[numb]);
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
    Serial.println("Temperature: "+String(temp));
    return (temp);

  }//end if/else
}//end gettempx

String buildstr(){
  
    Serial.println("sensor request");
    sensors.requestTemperatures(); //query all devices
    Serial.println("sensor request over");

    delay(0); // wifi stack breather
    String postStr = "";
  
    float temp = -127;
    float total = 0.0;
    
    for (uint8_t i = 1; i < (thermos + 1); i++) {

      temp = gettempx(i - 1);
      delay(0); // wifi stack breather
      
      if (isnan(temp)){ // reset on NAN
        resetFunc();
      }
      
      if (temp != -127){ // ignore on not-initialized
        postStr += "&field";
        postStr += String(i);
        postStr += "=";
        postStr += String(temp);
        total += temp;
        
      }//end temp-good
   

    }//end for


    delay(0); // wifi stack breather

    if ((0.0==total)&&(loops>5)){ //it's so unlikely that a sensor error is more likely --> reset
      resetFunc();
    }
    else{
      return(postStr);
    }
}

//connect to thingspeak, query sensors, upload data
void sendTemperatureTS(){
  WiFiClient client; //client object to connect to server
  delay(0); // wifi stack breather
  
  if (client.connect(server, 80)) {
    Serial.println("connected to wifi and server");

    String postStr = "";
    String apiKey = (char *)key;
    //Serial.println(apiKey);


    postStr = buildstr();

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
  //oh arduino, why don't you support "%s" ..
  Serial.println("sw version "+String(sw_version)+" with sensors "+String(sensorconfigname)+", loop Nr."+String(loops));

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
      //Serial.println("device " + String(i + 1) + " found");//don't show this random order
      Serial.println(strAddress(currentaddress)+" "+String(sensors.getTempCByIndex(i)));
      //Serial.println(nameforaddress(currentaddress));
    }
  }

  Serial.println("\naddress discovery over");

  loops++;
  if (loops > maxloops) {
    resetFunc();
  }

  
  if (uploadenabled){
    sendTemperatureTS();  
  }
  else{
    Serial.println(buildstr());
    Serial.println("Upload disabled for setup readout");
  }
  
  int count = myPeriodic;
  while (count--) //api delay f. thingspeak
    delay(1000);
    
}//end loop

