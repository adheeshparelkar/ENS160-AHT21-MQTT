# ENS160-AHT21-MQTT
ESP8266 MQTT For ENS160 + AHT21 Air Quality Sensors 
1. Refer for the the pinout diagram for your board to determine SDA and SCL pins. For WemosD1Mini  the pins are D1 (SCL) and D2 (SDA)
2. The ENS160-AHT21 shield I used requies 5V which I got directly from the WemosD1Mini.
3. The four connections are SCL, SDA, 5V and GND
4. Edit the Wifi SSID, Wifi Password, MQTT Server Address, MQTT Username and MQTT Password in mqtt.ino
5. Upload the sketch.
6. To troubleshoot open serial monitor at 9600bps to check status/errors. 
