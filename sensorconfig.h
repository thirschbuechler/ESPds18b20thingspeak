#ifndef sensorconfig_H
#define sensorconfig_H

#define myPeriodic 15 //in sec | Thingspeak pub is 15sec
#define ONE_WIRE_BUS 2  // DS18B20 on arduino pin2 corresponds to D4 on physical board

#define maxdisc 10 //for how many devices to look for by scanning the bus and print to serial
//(is unrelated to known devices list and temp gathering for upload)
#define thermos 2//how many sensors

const char* server = "api.thingspeak.com";// use ip 184.106.153.149 or api.thingspeak.com
const char* key = "";//write apikey oven 

//define known sensors here, order them into adresses ascending by field,
//use a 0x00, 0x00 ... dummy sensor if one field is empty
DeviceAddress T1 = {0x28, 0xF0, 0x0A, 0x77, 0x91, 0x06, 0x02, 0x6A};
DeviceAddress T2 = {0x28, 0xBB, 0x64, 0x77, 0x91, 0x14, 0x02, 0x29};
DeviceAddress *adresses [] = {&T1,&T2};

#endif
