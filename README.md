# ESP-Now-Sensor-system-with-WiFi-to-Internet
ESP-Now is used for battery operated Sensors sending to a Gateway which also is connected with WiFi to the Internet (at the same time).
Using ESP8266 and/or ESP32.

Soon to be added:
- Code for sensor (sending on ESP-NOW)
- Code for gateway (receiving on ESP-NOW and sending further via WiFi to the Internet, like Blynk, Thingspeak and MQTT.)
- System overview sketches
- Battery operation sketches (for sensors)

Low power and battery lifetime
Sensors operate in 'deep sleep' and is awaked every 5th minute to read a temp sensor (or other), activate wifi mudule and send the data using fast ESP-Now. The awake time is typically 200-250 ms. Due to very short 'wifi time' the most current/energy consuming part is the deep sleep current consumption
It is important with ESP board with LOW deep sleep currents. One of the better standard ones is the LOLIN D1 Mimi Pro V2.0.0 which has showed the longest life time spans in my tests. 

Typical life time performance of a Sensor (LOLIN D1 Mini Pro V2.0.0 and temp/humidity sensor) is 6 months using a 1200mAh LiPo battery and 12- months for a 2200mAh.
Using a small 55x80mm 5v solar panel and a TP4056 is enough for charging more than used, i.e. for a "continuous operation".
