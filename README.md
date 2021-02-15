# ESP-Now-Sensor-system-with-WiFi-to-Internet
ESP-Now is used for battery operated Sensors sending to a Gateway which also is connected with WiFi to the Internet (at the same time).
Using ESP8266 and/or ESP32.

Soon to be added:
- Code for sensor (sending on ESP-NOW)
- Code for gateway (receiving on ESP-NOW and sending further via WiFi to the Internet, like Blynk, Thingspeak and MQTT.)
- System overview sketches
- Battery operation sketches (for sensors)

Typical life time performance of a Sensor (LOLIN D1 Mini Pro and temp/humidity sensor) is 6 months using a 1200mAh LiPo battery and 12- months for a 2200mAh.
Using a small 55x80mm 5v solar panel and a TP4056 is enough for charging more than used, i.e. for a "continuous operation".
