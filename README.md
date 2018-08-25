# ESPds18b20thingspeak
ESP8266 based IOT ds18b20 temperature sensor bus  into thingspeak and highcharts html visualization

In case a OpenHub MQTT server, which would be the better option for home automation, you want only a standalone temperature monitor which doesn't require a local server but a remote-one in the internet, this might be for you: An Arduino-IDE based project to upload ds18b20 temperature sensor data to thingspeak, and visualizing it in a portable html file.

In sensorconfig.h make sure you have the correct port selected and after a sensordiscovery via a serial-monitor readout, input your sensors' addresses in the ascending order of the thingspeak channels' fields.
For migrating from a rasperry pi onewire ds18b20 bus to Arduino:
https://www.raspberrypi.org/forums/viewtopic.php?t=172776

Also, enter the write-key of your thingspeak-channel (private or public).

Wifi-config will be taken care of by Wifi-manager by Tzapu, once you've connected wirelessly to the AutoConnectAP it should redirect to 192.168.1.1 where you can input the wifi credentials to which it shall connect. May not work on eduroam or any PEAP/LEAP systems, unfortunately however. After a reboot or re-flash the settings don't get erased, unless you ask the Arduino environment to do so.

If you want visualization via a highcharts html file, open the supplied one and change $fieldnr and $readkey to match your thingspeak channel. $name can be choosen freely, it only reflects what is shown in the dropdown-menu for reloading older data into the visualization. Dollar-signs have to be removed.


##Libraries used:
Wifi-manager by Tzapu
ESP Library (ESP12-E for geekit-nodemcu-esp8266 board) for the Arduino IDE
Onewire, Dallastemperature

![nodemcudevkit_v1-0_io_edited](https://user-images.githubusercontent.com/8376996/43304379-2fe046a0-9174-11e8-8d53-1decdfd07fac.png)
