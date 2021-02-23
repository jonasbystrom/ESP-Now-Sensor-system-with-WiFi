# ESP-Now Sensor system with WiFi
Low power battery operated Sensors based on ESP8266 or ESP32, sending sensor data wireless using ESP-Now to an ESP8266/-32 Gateway connected to the Internet using WiFi (at the same time).

## Why this?
- Based on cheap standard ESP8266 (and ESP32) boards programmed in Arduino IDE
- 1 year operation on 18650 battery
- For-ever operation with a small solar panel (80x55mm)
- ESP-Now receiver (gateway) connected to WiFi/Internet - at same time
- Longer range than WiFi, no need for external antennas

## Typical usage 
Battery and/or solar powered sensors and weather stations connected to home automations, to Blynk apps, to Thingspeak, MQTT, etc.

## Soon to be added
- Code template for sensor node (sending on ESP-NOW)
- Code example for sensor using an SHT30 temp sensor
- Code template for gateway (receiving on ESP-NOW and sending further via WiFi to the Internet, like Blynk, Thingspeak and MQTT.)
- System overview sketch
- Battery operation sketches (for sensors)


## Life time

Typical life time performance of a Sensor with LOLIN D1 Mini Pro V2.0.0 and a SHT30 temp/humidity sensor:
- 6+ months using a 1200mAh LiPo battery
- 12 months using a 2200mAh Li-Ion battery (not yet passed 12 months, but on its way)
- Continuous operation using a small 80x55mm 5v solar panel and a TP4056 charger with a 1200 or 2200 mAh battery.


## Sensor description

While HW is standard ESP boards, the sensor code is optimized for lowest power consumption possible. Sensors operate in Deep Sleep mode in 5 minute cycles. The WiFi module is active in the region of 60 ms.

The ESP-Now messages are "unnecessary" long and not just a few bytes as it would take to send sensor data. However, I have standardized on a more general and longer format as the extra time to transmit has quite a low impact on energy consumption and battery lifetime.

The energy consumption can be divided into 3 classes:
1. Deep Sleep period: 300 secs, 50-200uA. Some boards draw much more in deep sleep. 
2. Wake time reading sensor etc with WiFi OFF: 100-500ms, 15-30mA. Fast sensors are preferred. 
3. Wake time sending data with WiFi ON: 60ms, 70-150mA. 

From the above typical values, it is realized the major energy consumption is in the Deep Sleep mode why it is important to use ESP boards with low deep sleep currents. The LOLIN D1 Mini Pro V2.0.0 is the standard ESP board with the longest battery lifetime I have tested.

Best sensor lifetime performance has been observed using a LOLIN SHT30 Temp and Humid sensor, with a modified driver library. The DS18B20 temp sensor comes very close by. The BME280, draws a bit more current with a noticeable shorter lifetime. DHT11, DHT21 and DHT22 all consume more power, either by higher standby current or by longer reading times, resulting in shorter battery lifetime. They also produce much more false readings and easily gets over-saturated in humid outdoors conditions.

_Note 1. I have a FireBeetle ESP8266 IoT Microcontroller board under test which looks very promising with potentially even longer battery performance._

_Note 2. It would be further possible to reduce the deep sleep current by using ESP modules with no USB and LDO chips, or by disabling built-in LDO's replacing with more efficient external ones, etc. However, i haven't tried this very much but instead focused on an optimal SW solution using standard boards._

![](https://github.com/jonasbystrom/ESP-Now-Sensor-system-with-WiFi/blob/main/img/esp-now-temp-sensor-with-solar-panel.png)


## Gateway summary

A gateway receives the sensor data and can send further to any service on the Internet such as Blynk, Thingspeak or MQTT. Gateways are based on ESP8266 or ESP32 with small code changes in ESP-Now and WifI API's. The Gateway should have both ESP-Now and WiFi actived at same time and the ESP's can only manage this if they are on the same channel. And it does only seem work to for channel 1, due to some implementation limitation in the ESP's. This means the WiFi Router must be set to channel 1 on the 2.4GHz band. It can not operate on "auto channel", it has to be set to channel 1 fixed.

By specification a Gateway can manage max 20 sending sensors on ESP-Now. It is fully possible to use several Gateways in parallel on the same channel if there is a need for more than 20 sensors. I have been running with 3 gateways at the same time without any collisions or problems.

## System summary

A system comprise of up to 20 sensors and 1 gateway. Sensors send to the MAC-adress of the gateway. It is possible, and preferred, to use a software defined MAC-address in the gateway and not the default hardware MAC address. As this allows for an exchange of the gateway hardware without any problems. The ESP's support this.
Each system should have it's own MAC address to avoid collisions with other systems.
It is said ESP-Now has "3 times longer access range" than WiFi. I have never tested this but I have noted long enough access range for my sensors in my installations. The communication stability is very good. I have never experienced any communication losses in all normal cases. I havent tested this very much but i typically get 99 or 100 sucessful transmissions out of 100 tries.  
