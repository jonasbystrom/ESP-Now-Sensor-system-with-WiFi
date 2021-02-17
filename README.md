# ESP-Now Sensor system with WiFi
Low power battery operated Sensors based on ESP8266 or ESP32, sending sensor data using ESP-Now to a ESP Gateway which also is using WiFi and conncted to the Internet (at the same time).

## Soon to be added:
- Code for sensor (sending on ESP-NOW)
- Code for gateway (receiving on ESP-NOW and sending further via WiFi to the Internet, like Blynk, Thingspeak and MQTT.)
- System overview sketch
- Battery operation sketches (for sensors)

## Sensor summary:

Sensors operate in 'deep sleep' mode and wake up every 5th minute to read a temp sensor (or other), activate the wifi module and send the data to a Gateway using the fast ESP-Now. Since sleep time is 5 mins and the typical wake time is only 200-250ms, there is more energy/current consumed during the deep sleep cycle than during activation. It is therefore important to use ESP boards with LOW deep sleep currents for optimal life time performance. 
One of the better standard ESP8266 boards is the LOLIN D1 Mini Pro V2.0.0 which has showed the longest life time spans during my tests. Other ways would be to use naked ESP modules with even lower current consumptions.  
It is also important to use sensors with low stand-by current consumption. Either low in stand-by or enabled only when used, every 5th minute. LOLIN SHT30 and DS18B20 are both low in current consumption.

## Life time:

Typical life time performance of a Sensor (LOLIN D1 Mini Pro V2.0.0 and a SHT30 temp/humidity sensor) is 6 months using a 1200mAh LiPo battery and targeting 12 months for a 2200mAh.
Using a small 80x55mm 5v solar panel and a TP4056 charger with a 1200 or 2200 mAh battery is well enough for "continuous operation".

## Gateway summary:

A gateway receives the sensor data and can send further to any service on the Internet such as Blynk, Thingspeak or MQTT. Gateways are based on ESP8266 or ESP32 with small code changes in ESP-Now and WifI API's. The Gateway should have both ESP-Now and WiFi actived at same time and the ESP's can only manage this if they are on the same channel. And it does only seem work to for channel 1, due to some implementation limitation in the ESP's. This means the WiFi Router must be set to channel 1 on the 2.4GHz band. It can not operate on "auto channel", it has to be set to channel 1 fixed.

By specification a Gateway can manage max 20 sending sensors on ESP-Now. It is fully possible to use several Gateways in parallel on the same channel if there is a need for more than 20 sensors. I have been running with 3 gateways at the same time without any collisions or problems.

## System summary:

A system comprise of up to 20 sensors and 1 gateway. Sensors send to the MAC-adress of the gateway. It is possible, and preferred, to use a software defined MAC-address in the gateway and not the default hardware MAC address. As this allows for an exchange of the gateway hardware without any problems. The ESP's support this.
Each system should have it's own MAC address to avoid collisions with other systems.
It is said ESP-Now has "3 times longer access range" than WiFi. I have never tested this but I have noted long enough access range for my sensors in my installations. The communication stability is very good. I have never experienced any communication losses in all normal cases. I havent tested this very much but i typically get 99 or 100 sucessful transmissions out of 100 tries.  
