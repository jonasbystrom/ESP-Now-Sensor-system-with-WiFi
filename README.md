# ESP-Now Sensor system with WiFi
Low power battery operated Sensors based on ESP8266 or ESP32, sending sensor data using ESP-Now to a ESP Gateway which also is using WiFi and conncted to the Internet (at the same time).

Soon to be added:
- Code for sensor (sending on ESP-NOW)
- Code for gateway (receiving on ESP-NOW and sending further via WiFi to the Internet, like Blynk, Thingspeak and MQTT.)
- System overview sketch
- Battery operation sketches (for sensors)

Sensor summary:

Sensors operate in 'deep sleep' mode and wake up every 5th minute to read a temp sensor (or other), activate the wifi module and send the data to a Gateway using the fast ESP-Now. Since sleep time is 5 mins and the typical wake time is only 200-250ms, there is more energy/current consumed during the deep sleep cycle than during activation. It is therefore important to use ESP boards with LOW deep sleep currents for optimal life time performance. 
One of the better standard ESP8266 boards is the LOLIN D1 Mini Pro V2.0.0 which has showed the longest life time spans during my tests. Other ways would be to use naked ESP modules with even lower current consumptions.  
It is also important to use sensors with low stand-by current consumption. Either low in stand-by or enabled only when used, every 5th minute. LOLIN SHT30 and DS18B20 are both low in current consumption.

Life time:

Typical life time performance of a Sensor (LOLIN D1 Mini Pro V2.0.0 and a SHT30 temp/humidity sensor) is 6 months using a 1200mAh LiPo battery and targeting 12 months for a 2200mAh.
Using a small 80x55mm 5v solar panel and a TP4056 charger with a 1200 or 2200 mAh battery is well enough for "continuous operation". 

