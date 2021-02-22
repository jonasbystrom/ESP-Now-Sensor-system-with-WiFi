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

## Sensor description

While HW is standard ESP boards, the sensor code is optimized for lowest power consumption possible. Sensors operate in Deep Sleep mode in 5 minute cycles. The WiFi module is active less than 100 ms, more likely in the range of 50 ms.

My ESP-Now messages are "unnecessary" long as it would only require a few bytes to send sensor data. However, I have standardized on a more general and longer format as the extra time to transmit has low impact on energy consumption and battery lifetime.

The energy consumption can be divided into 3 classes. 1. Deep Sleep period (100-500uA or much more) 2. Wake time reading sensor etc with WiFi off (15-30mA), 3. Sending data with WiFi on (70-150mA). With a deep sleep period of 5 mins between sensor readings and transmissions, it is the Deep Sleep period which is the major energy consumer for most standard ESP boards. Why it is very important to choose a board, or module, with as low deep sleep current as possible. D1 Mini Pro V2.0 is the best standard board I have tested. 

(I have a FireBeetle ESP8266 IoT under test and evaluation which looks promising. Building your own sensor with a naked ESP-module without USB drivers and LDO would lower the deep sleep current potential. It would also be possible to disable sensors by SW or HW during sleep, but I have only tested this very little and with no resulting battery lifetime improvements.)

Best lifetime performance has been observed with the LOLIN SHT30 temp and humid sensor, using a modified library driver. The DS18B20 temp sensor comes very close by. Then BME280, which consumes a bit more current with a noticeable shorter lifetime. DHT11, -21 and -22 all consumes more power with shorter battery lifetime and also produce much more false readings and/or stops reading, especially for outdoor use.

![](https://github.com/jonasbystrom/ESP-Now-Sensor-system-with-WiFi/blob/main/img/esp-now-temp-sensor-with-solar-panel.png)


## Life time

Typical life time performance of a Sensor with LOLIN D1 Mini Pro V2.0.0 and a SHT30 temp/humidity sensor:
- 6 months using a 1200mAh LiPo battery
- 12 months using a 2200mAh Li-Ion battery (not yet passed 12 months, but on its way)
- Continuous operation using a small 80x55mm 5v solar panel and a TP4056 charger with a 1200 or 2200 mAh battery.

## Gateway summary

A gateway receives the sensor data and can send further to any service on the Internet such as Blynk, Thingspeak or MQTT. Gateways are based on ESP8266 or ESP32 with small code changes in ESP-Now and WifI API's. The Gateway should have both ESP-Now and WiFi actived at same time and the ESP's can only manage this if they are on the same channel. And it does only seem work to for channel 1, due to some implementation limitation in the ESP's. This means the WiFi Router must be set to channel 1 on the 2.4GHz band. It can not operate on "auto channel", it has to be set to channel 1 fixed.

By specification a Gateway can manage max 20 sending sensors on ESP-Now. It is fully possible to use several Gateways in parallel on the same channel if there is a need for more than 20 sensors. I have been running with 3 gateways at the same time without any collisions or problems.

## System summary

A system comprise of up to 20 sensors and 1 gateway. Sensors send to the MAC-adress of the gateway. It is possible, and preferred, to use a software defined MAC-address in the gateway and not the default hardware MAC address. As this allows for an exchange of the gateway hardware without any problems. The ESP's support this.
Each system should have it's own MAC address to avoid collisions with other systems.
It is said ESP-Now has "3 times longer access range" than WiFi. I have never tested this but I have noted long enough access range for my sensors in my installations. The communication stability is very good. I have never experienced any communication losses in all normal cases. I havent tested this very much but i typically get 99 or 100 sucessful transmissions out of 100 tries.  
