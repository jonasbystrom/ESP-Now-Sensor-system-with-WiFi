
## Building a system
You can follow these steps to build a working system.

## Sensor
- Program 1 (or more) ESP8266 or ESP32 with the sketch in espnow_sensor. Use ARDUINO IDE.
  - Remember to set the appropriate #define USE_ESP8266 or USE_ESP32.
  - If using several sensors, please remember to set unique UNIT (ID) for each of them. 
- HW configs:
  - Connect RST to D0 (GPIO16) on the ESP8266 board. 
    - Only needed for ESP8266. To be able to wake up from Deep Sleep.
    - On a WEMOS/LOLIN D1 Mini Pro V2.0.0 board you can do this by soldering the pad "SLEEP" (which connects RST to D0). 
  - Connect BAT to A0 on the ESP8266 board.
    - Only needed for ESP8266.  
    - If you are using a battery and want to read the battery level. 
  - Connect a battery to the ESP board
    - _You can of course run the sensor on USB 5V if you want. But the main idea with ESP-Now is low power which is needed for battery operations._
  - Connect a Temp sensor to the ESP board.
    - Either use a SHT30 or DS18B20 sensor with provided code, or add your own code for any other sensor type. 
       
  _As a first test you may skip these HW changes and just run without a temp sensor but send dummy temp data and with "manual" resets emulating a wake from deep sleep. You can also comment out the deep sleep code and just add a "delay(1000 * 10); ESP.restart();" for a 10 secs delay between restarts and readings. This is of course only for test as it will still consume lots of energy._

## Gateway
- Program 1 ESP8266 or ESP32 with the sketch in espnow_gateway. Use ARDUINO IDE.
  - Remember to set the appropriate #define USE_ESP8266 or USE_ESP32.
- The ESP should normally run on USB 5V.
- No HW modifications needed.

## Router
- Set the 2.4GHz band of your router to "fixed" channel 1
  - _I personally use a separate Router defining a sepatate WiFi network (SSID) which works fixed on channel 1._
