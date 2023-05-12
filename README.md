# weather-station

Weather station for esp8266. This is made for two devices communicating - **transmitter** (station) and **receiver** (AP). 
Each station has a sensor BMP-280 that is used to measure the temperature and pressure. The wireless channel to send measurements is set through nRF24.

To upload to the microcontroller, erase flash or delete the built files, use the script **./run**, followed by the station name.

---
Example of flashing a station that acts as a receiver.
```
./run receiver
```
Erasing flash and uploading
```
./run receiver erase
```
Cleaning and uploading
```
./run receiver clean
```
You can also just use **make** to execute any command, but you need to specify which Makefile to use
```
make -f Makefile-receiver clean
```
