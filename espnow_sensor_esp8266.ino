#define SKETCH_NAME     "ESP-Now Sensor ESP8266"
#define SKETCH_VERSION  "2021-12-30"
#define SKETCH_ABOUT    "Temp sensor, Deep Sleep, ESP-Now, ESP8266."

//
//  This is a template code for an ESP-Now sensor for ESP8266. It Assumes Arduino IDE.
//  ( A later version will also include the small differencies needed for ESP32.)
//
//  The ESP-Now sensor sends messages on ESP-Now to a MAC address of a Gateway which is 
//  connected to WiFi and Internet.
//
//  A sensor unit consists of; ESP8266 board, a Temp Sensor and a LiPo/Li-Ion battery cell. 
//  An optional Solar panel and Solar panel manager/Battery charger can be connected.
//
//  The ESP8266 operates in deep sleep mode and wakes every 5 mins, takes a sensor reading, 
//  transmits the data using ESP-Now to an ESP-Now Gateway and then returns to deep sleep.
//  The design is aimed for Battery Cell and/or Solar Panel powered units with as low power 
//  consumption as possible, using standard ESP8266 boards. No special, or modified, ultra low 
//  power ESP8266 boards are required.
//
//  ESP-Now is much faster than ordinary WiFi (TCP/IP), reducing the "sending energy" dramatically. 
//  The drawback is that ESP-Now is a proprietary protocol of Espressif and is only implemented 
//  (what i know) for ESP MCU's.
//
//  The wakeup time of the MCU is heavily dependent on the temp sensor used. Some temp sensors 
//  will return sensor data within 100 ms or shorter, some requires 500 ms or more. This varies 
//  with sensor type/manufacturer (spec) and samples. Specs often claim a wake-up or stabilization
//  time of upto 2 secs, but most sensors can return a stable (?) measurement in 200-500 ms typically.
//
//  (Generally, SHT30 and DS18B20 temp sensors perform well and decently fast. DHT11/21/22 are slower 
//  and less reliant in my testes. I dont use them in real life.)
//
//
//  CREDITS:
//  This code is based on code from:
//  - Anthony Elder @ https://github.com/HarringayMakerSpace/ESP-Now/blob/master/espnow-sensor-bme280/espnow-sensor-bme280.ino
//  With important info from:
//  - Erik Bakke @ https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-2/
//  And great info from:
//  - ArduinoDIY @ https://arduinodiy.wordpress.com/2020/01/18/very-deepsleep-and-energy-saving-on-esp8266/

//  HARDWARE:
//  - D1 Mini Pro v2.0 (or, D1 Mini v3.1.0)
//  - SHT30 or DS18B20 temp sensor
//  - 3.7V LiPo/Li-Ion battery with JST connector. Flat 1200mAh, 18650 2200mAh or similar.
//  - Optional: Solar panel and solar panel charger board
//
//  CONNECTIONS:
//
//  D1 Mini Pro V2.0.0:
//  ===================
//  BAT --- A0          (solder/short the "BAT-A0" pad for battery level measurement)
//  RST --- D0/GPIO16   (solder/short the "SLEEP" pad)
//
//
//  D1 Mini SHT30 (If using WEMOS/LOLIN boards, just plug them together.)
//  ======= =====
//  GND     GND
//  3V3     VCC
//  D1/SCL  SCK
//  D2/SDA  SDA
//
//  or, if using a DS18B20
//
//  D1 Mini - DS18B20 (with an internal resistor)
//  =======   =======
//  GND       G (GND)
//  3V        V (VCC)
//  D3
//  D4        S (DATA)
//
//
//  HISTORY:
//  ========
//  2021-12-30  Cleaned for public publish on GitHub
//


#include <ESP8266WiFi.h>
#include <espnow.h>


// ------------------------------------------------------------------------------------------
// ESP-NOW SYSTEM CONFIGS
// ------------------------------------------------------------------------------------------
#define WIFI_CHANNEL        1     // Must be 1. (!IMPORTANT)
                                  // ESP-Now can work on other channels, but the receiving ESP Gateway must operate on
                                  // channel 1 in order to also work for TCP/IP WiFi.
                                  // It has been reported to work also for other Channels, but I have only succeeded on ch 1.

uint8_t Gateway_Mac[] = {0x02, 0x10, 0x11, 0x12, 0x13, 0x14};
                                  // MAC Address of the remote ESP Gateway we send to.
                                  // This is the "system address" all Sensors send to and the Gateway listens on.
                                  // (The ESP Gateway will set this "soft" MAC address in its HW. See Gateway code for info.)

typedef struct sensor_data_t {    // Sensor data format for sending on ESP-Now to Gateway
  int           unit;             // Unit no to identy which sensor is sending
  float         temp;             // Temperature (C)
  float         humidity;         // Humidity (%)
  float         baro_pressure;    // Barometric pressure (hPa)
  int           lux;              // Light sensor data (lux)
  float         Vbat;             // Battery voltage level (V)
  char          ID[80];           // Any clear text to identify the unit
  int           wakeTimeMS;       // Sensor wake time until sending data
  unsigned long updated;          // Epoch time when received by Gateway. Set by gateway/receiver. (Not used by sensor, but part of struct for convenience reasons.)
} sensor_data_t;


// -----------------------------------------------------------------------------------------
// ESP SENSOR CONFIGS
// -----------------------------------------------------------------------------------------
#define UNIT                1       // Sensor unit ID to identify THIS unit for the receiving gateway. Recommended to use [1 -20]

#define USE_ESP8266                 // Select (uncomment) one of ESP8266 or ESP32. Not supported at the moment. Only ESP8266 supported for now.
//#define USE_ESP32

#define USE_SHT30                   // Select (uncomment) the temp sensor type in use
//#define USE_DS18B20

#define DEBUG_LOG                   // Enable (uncomment) to print debug info. Disabling (comment) debug output saves some 4-5 ms ...

//ADC_MODE(ADC_VCC);                // Uncomment to Enable reading Vcc (if board cannot read VBAT).

#define SLEEP_SECS        5*60-8    // [sec] Sleep time between wake up and readings. Will be 5 mins +/- 8 secs. Varying to avoid transmit collusions.
#define MAX_WAKETIME_MS   1000      // [ms]  Timeout until forced gotosleep if no sending success 



#ifdef USE_SHT30
// -----------------------------------------------------------------------------------------
//  WEMOS SHT30 Temp and Humidity sensor
// -----------------------------------------------------------------------------------------
#include "Wire.h"
#define ADDR              0x45      // I2C address of SHT30.
#endif


#ifdef USE_DS18B20
// -----------------------------------------------------------------------------------------
// DS18B20 - TEMP SENSOR
// -----------------------------------------------------------------------------------------
// Temp probe is connected with onewire interface.
//
#include <OneWire.h>
#include <DallasTemperature.h>

// Data wire is plugged into port D3 & D4 on the ESP8266
#define ONE_WIRE_BUS            D4                // Change to other port if wanted/needed
#define TEMPERATURE_PRECISION   12                // [9-12]. 12 => resolution of 0.0625 C
                      /*  12-bit precision:
                          1-bit for sign, 7-bit for integer part, and 4-bit for fractional part (4-digit after decimal point)
                          Temperature range: xxx.0000 C to xxx.9375 C in 0.0625 C discrete step.
                      */        
OneWire oneWire (ONE_WIRE_BUS);                   // Setup a oneWire instance to communicate with any OneWire devices
DallasTemperature tempSensor(&oneWire);           // Pass our oneWire reference to Dallas Temperature.
DeviceAddress tempDeviceAddress;                  // We'll use this variable to store a found device address

#endif


// -----------------------------------------------------------------------------------------
//  BATTERY LEVEL CALIBRATION
// -----------------------------------------------------------------------------------------
#define CALIBRATION         4.21 / 4.35                       // Measured V by multimeter / reported (raw) V 
                                                              // (Set to 1 if no calibration is needed/wanted)
#define VOLTAGE_DIVIDER     (130+220+100)/100 * CALIBRATION   // D1 Mini Pro voltage divider to A0. 
                                                              // May be different for other boards.


// -----------------------------------------------------------------------------------------
// GLOBALS
// -----------------------------------------------------------------------------------------
sensor_data_t sensorData;
volatile boolean messageSent;     // flag to tell when message is sent out and we can safely goto sleep


#ifdef USE_SHT30
// -----------------------------------------------------------------------------------------
byte sht30_get(float &temp, float &humid)
// -----------------------------------------------------------------------------------------
// Read the SHT30 temp/humid sensor.
// Code is from WEMOS_sht3x.cpp - modified by reducing delays and adjusting types.
{
  unsigned int data[6];

  temp = NAN;
  humid = NAN;

  Wire.beginTransmission(ADDR);           // Start I2C Transmission
  Wire.write(0x2C);                       // Send measurement command
  Wire.write(0x06);
  if (Wire.endTransmission() != 0)        // Stop I2C transmission
    return 1;

  delay(100);         // 100 ms sensor stabilization time. Shorter than spec, but works OK.  Can be tweaked if needed.
                      // (I have seen Tasmota code to read SHT30 a bit different and as it seems even faster - would be interesting to try ...)

  Wire.requestFrom(ADDR, 6);              // Read the 6 data bytes
  for (int i = 0; i < 6; i++) {
    data[i] = Wire.read();
  };

  delay(50);                              // Let it stabilize. Can be tweaked
  if (Wire.available() != 0)
    return 2;
  // Convert the data
  temp = ((((data[0] * 256.0) + data[1]) * 175) / 65535.0) - 45;
  //fahrenheit version: temp = (temp * 1.8) + 32;
  humid = ((((data[3] * 256.0) + data[4]) * 100) / 65535.0);

  return 0;
}
#endif


// -----------------------------------------------------------------------------------------
void setup()
// -----------------------------------------------------------------------------------------
{
  // Disable WiFi until we shall use it, to save energy
  WiFi.persistent( false );         // Dont save WiFi info to Flash - to save time
  WiFi.mode( WIFI_OFF );            // Wifi OFF - during sensor reading - to save current/power
  WiFi.forceSleepBegin();
  delay( 1 );                       // Necessary for the OFF to work. (!IMPORTANT)

  #ifdef USE_SHT30
  Wire.begin();                     // Prepare the I2C communication
  #endif

  #ifdef DEBUG_LOG
  Serial.begin(115200);
  while (!Serial) {};
  Serial.println("\n\nStart");
  #endif

  #ifdef USE_DS18B20
  // Init sensor/bus
  //pinMode(ONE_WIRE_BUS, OUTPUT);     // use this if using PARASITE mode of DS18B20 (vcc from data line, and probably a pull up resistor ...)
  tempSensor.begin();
  tempSensor.getAddress(tempDeviceAddress, 0);
  tempSensor.setResolution(tempDeviceAddress, TEMPERATURE_PRECISION);
  tempSensor.requestTemperatures();
  #endif

  // read battery voltage
  int raw = analogRead(A0);
  sensorData.Vbat = raw * VOLTAGE_DIVIDER / 1023.0;
          // Alternative. If cannot read Battery level on your board, Read Vcc instead
          // const float calVal = 0.001108;     // 3.27/2950=0.001108. Vcc 3.27 on Multimeter, 2950 from getVcc()
          // sensorData.Vbat = ESP.getVcc()/1023; // * calVal;

  #ifdef DEBUG_LOG
  Serial.print("Battery voltage:"); Serial.print(sensorData.Vbat); Serial.println(" V");
  #endif

  // compile message to send
  strcpy (sensorData.ID, SKETCH_NAME);
  strcat (sensorData.ID, " - ");
  strcat (sensorData.ID, SKETCH_VERSION);

  sensorData.unit = UNIT;

  // Read the temp sensor
  #ifdef USE_SHT30
  if (sht30_get(sensorData.temp, sensorData.humidity) == 0) {
    // reading ok   
  } else {
    // SHT30 reading error, use -999 as "invalid data". (Maybe it is better to use NAN instead (?))
    sensorData.temp = -999;
    sensorData.humidity = -999;
  }
  #elif defined USE_DS18B20
  sensorData.temp     = tempSensor.getTempC(tempDeviceAddress);
  #endif


  // WiFi ON                    (This step seems not to be necessary, but anyway ...)
  WiFi.forceSleepWake();
  delay( 1 );

  // Set up ESP-Now link ---------------------------
  WiFi.mode(WIFI_STA);          // Station mode for esp-now sensor node
  WiFi.disconnect();
  #ifdef DEBUG_LOG
  Serial.printf("My HW mac: %s", WiFi.macAddress().c_str());
  Serial.println("");
  Serial.printf("Sending to MAC: %02x:%02x:%02x:%02x:%02x:%02x", Gateway_Mac[0], Gateway_Mac[1], Gateway_Mac[2], Gateway_Mac[3], Gateway_Mac[4], Gateway_Mac[5]);
  Serial.printf(", on channel: %i\n", WIFI_CHANNEL);
  #endif

  // Initialize ESP-now ----------------------------
  if (esp_now_init() != 0) {
    #ifdef DEBUG_LOG
    Serial.println("*** ESP_Now init failed. Going to sleep");
    #endif
    delay(100);
    gotoSleep();
  }
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);
  esp_now_add_peer(Gateway_Mac, ESP_NOW_ROLE_SLAVE, WIFI_CHANNEL, NULL, 0);
  esp_now_register_send_cb([](uint8_t* mac, uint8_t sendStatus) {
    // callback for message sent out
    messageSent = true;         // flag message is sent out - we can now safely go to sleep ...
    #ifdef DEBUG_LOG
    Serial.printf("Message sent out, sendStatus = %i\n", sendStatus);
    #endif
  });

  messageSent = false;

  // Send message -----------------------------------
  #ifdef DEBUG_LOG
  Serial.println("Message Data: " + \
                  String(sensorData.ID) + ", Unit:" + \
                  String(sensorData.unit) + ", Temp:" + \
                  String(sensorData.temp) + "C, Hum: " + \
                  String(sensorData.humidity) + "%, Lux: " + \
                  String(sensorData.lux) + ", Vbat:" + \
                  String(sensorData.Vbat) \
                );
  #endif
  uint8_t sendBuf[sizeof(sensorData)];          // create a send buffer for sending sensor data (safer)
  sensorData.wakeTimeMS = millis();             // set wake time until now
  memcpy(sendBuf, &sensorData, sizeof(sensorData));
  uint16_t result = esp_now_send(NULL, sendBuf, sizeof(sensorData));
  #ifdef DEBUG_LOG
  Serial.print("Wake: "); Serial.print(sensorData.wakeTimeMS); Serial.println(" ms");
  Serial.print("Sending result: "); Serial.println(result);
  #endif
}

// -----------------------------------------------------------------------------------------
void loop()
// -----------------------------------------------------------------------------------------
{
  // Wait until ESP-Now message is sent, or timeout, then goto sleep
  if (messageSent || (millis() > MAX_WAKETIME_MS)) {
    gotoSleep();
  }
}


// -----------------------------------------------------------------------------------------
void gotoSleep()
// -----------------------------------------------------------------------------------------
{
  int sleepSecs;

  #ifdef USE_ESP8266
  sleepSecs = SLEEP_SECS + ((uint8_t)RANDOM_REG32 / 16);  // add random time to avoid traffic jam collisions
  #ifdef DEBUG_LOG
  Serial.printf("Up for %i ms, going to deep sleep for %i secs ...\n", millis(), sleepSecs);
  #endif

  ESP.deepSleep(sleepSecs * 1000000, RF_NO_CAL);
  delay (10);                                             // good convention with delay after call to deep sleep.

  // Never return here - ESP will be reset after deep sleep
  #endif

  #ifdef USE_ESP32
  // later ...
  #endif
}
