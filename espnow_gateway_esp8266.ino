#define SKETCH_NAME    "ESP-NOW GATEWAY 8266"
#define SKETCH_VERSION "2021-12-30"
#define SKETCH_ABOUT   "ESP8266 ESP-Now Gateway template code and demonstrator of simultaneous ESP-Now and WiFi."

/*
 * This is a template code for an ESP-Now Gateway for ESP8266. It assumes Arduino IDE.
 * (A later version will also include the small differencies needed for an ESP32.)
 * 
 * The Gateway receives messages on the Espressif proprietary protocol ESP-Now. It also provides a simple WEB server
 * displaying the received temperature data from all sensors. This is to demonstrate a single ESP8266 is capable of 
 * communicating on ESP-Now and 2.4 GHz Wifi simoultaneously. One restriction apply, this only works on WiFi channel 1 
 * and you must therefore set the router to "fixed" channel 1.
 * 
 * In order to perform the demonstration you will need: 
 * - 1x ESP8266 programmed with this sketch, acting as Gateway (to the WiFi/internet), and 
 * - 1-20x ESP8266 programmed with the espnow_sensor sketch and equipped with a temperature sensor (or anything else
 * you connect and modify the code for).
 * 
 * This is a simple demo sketch but fully working. It demostrates simultaneous WiFi and eSP-Now on a ESP 8266.
 * You can add other Internet services like MQTT, Blynk, ThingSpeak etc. (I have been running this for 1-2 years with 
 * those services.) 
 * 
 * CREDITS:
 * Code related to ESP-Now is partly based on code from:
 *  - Anthony Elder @ https://github.com/HarringayMakerSpace/ESP-Now/blob/master/espnow-sensor-bme280/espnow-sensor-bme280.ino
 *  With important info from:
 *  - Erik Bakke @ https://www.bakke.online/index.php/2017/05/21/reducing-wifi-power-consumption-on-esp8266-part-2/
 *  And great info from:
 *  - ArduinoDIY @ https://arduinodiy.wordpress.com/2020/01/18/very-deepsleep-and-energy-saving-on-esp8266/
 */

//  HARDWARE
//  - D1 Mini Pro V2.0.0 (But any ESP8266 board should work)
//
//  HISTORY:
//  2020-10-27  First public version
//


#include <ESP8266WiFi.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WebServer.h>
#include <espnow.h>


// ------------------------------------------------------------------------------------------
// ESP-NOW SYSTEM CONFIGS
// ------------------------------------------------------------------------------------------
//
// This is the MAC address to be installed (sensors shall then send to this MAC address)
uint8_t GatewayMac[] =      {0x02, 0x10, 0x11, 0x12, 0x13, 0x14};

    /*  Note, the Gateway listens on this MAC address. All sensors shall send to this MAC address.
     *  You can set any MAC address of your choice according to this table of "free-to-use local MAC addresses":
     *    {0x02, any, any, any, any, any}
     *    {0x06, any, ...}
     *    {0x0A, any, ...}
     *    {0x0E, any, ...}
     *  
     *  Further, it would be possible to use the built-in HW-MAC address of the ESP8266. But, in case you would need to  
     *  change the ESP (for any reasons) you would then need to update ALL sensors to the same new MAC address. 
     *  It is therefore better to use a soft MAC defined by code. This will be installed in any new ESP HW you may use.
     *  Just remeber to set a new MAC address for every new system (gateway+sensors) you install in paralell. *  
     */
 
// ESP-Now message format. Sensor data is transmitted using this struct.
typedef struct sensor_data_t {   
  int           unit;               // Unit no to identy which sensor is sending
  float         temp;               // Temperature (C)
  float         humidity;           // Humidity (%)
  float         baro_pressure;      // Barometric pressure (hPa)
  int           lux;                // Light sensor data (lux)
  float         Vbat;               // Battery voltage level (V)
  char          ID[80];             // Any clear text to identify the unit
  int           wakeTimeMS;         // Sensor wake time until sending data
  unsigned long updated;            // Epoch time when received by Gateway. Set by gateway/receiver. (Not used by sensor, but part of struct for convenience reasons.)
} sensor_data_t;


// ------------------------------------------------------------------------------------------
// ESP-NOW GATEWAY CONFIGS
// ------------------------------------------------------------------------------------------
// Router WiFi Credentials (runnning on 2.4GHz and Channel=1)
#define SOFTAP_SSID       "...YOUR SSID Network Name..."
#define SOFTAP_PASS       "...YOUR Network Password ..."
#define CHANNEL           1

#define UNITS             20          // No of esp-now sensor units supported to receive from. ESP-Now has a maximum of 20


// ------------------------------------------------------------------------------------------
// GLOBALS
// ------------------------------------------------------------------------------------------
ESP8266WiFiMulti wifiMulti;

sensor_data_t bufSensorData;          // buffer for incoming data
sensor_data_t sensorData[UNITS+1];    // buffer for all sensor data


// ------------------------------------------------------------------------------------------
// ESP-NOW functions
// ------------------------------------------------------------------------------------------
//
// This is a callback function from ESP (?), anyway (!IMPORTANT)
void initVariant()
{
  wifi_set_macaddr(SOFTAP_IF, &GatewayMac[0]);       //8266 code
  wifi_set_macaddr(STATION_IF, &GatewayMac[0]);      //8266 code
}

// Callback when data is received from any Sender
void OnDataRecv(uint8_t *mac_addr, uint8_t *data, uint8_t data_len)
{
  digitalWrite (LED_BUILTIN, !HIGH);                // Led ON
  
  char macStr[24];
  snprintf(macStr, sizeof(macStr), " %02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.print("\nData received from: "); Serial.println(macStr);
  memcpy(&bufSensorData, data, sizeof(bufSensorData));

  // Print data
  Serial.print ("ID: ");
  Serial.print (bufSensorData.ID);
  Serial.println ("");
  Serial.print ("Unit: ");
  Serial.print (bufSensorData.unit);
  Serial.print ("   Temp: ");
  Serial.print (bufSensorData.temp);
  Serial.print ("   Humidity: ");
  Serial.print (bufSensorData.humidity);
  Serial.print ("   Baro: ");
  Serial.print (bufSensorData.baro_pressure);
  Serial.print ("   Lux: ");
  Serial.print (bufSensorData.lux);
  Serial.println ("");    
  Serial.print ("Vbat: ");
  Serial.print (bufSensorData.Vbat);
  Serial.print ("   Wake: ");
  Serial.print (bufSensorData.wakeTimeMS);
  Serial.println ("");

  // Store data
  int i = bufSensorData.unit;
  if ( (i >= 1) && (i <= UNITS) ) {
    memcpy(&sensorData[i], data, sizeof(bufSensorData));
  };
  
  digitalWrite (LED_BUILTIN, !LOW);                 // Led OFF}
}


// ------------------------------------------------------------------------------------
// WEB server functions
// ------------------------------------------------------------------------------------
ESP8266WebServer server(80);

void handleRoot() {
  digitalWrite (LED_BUILTIN, !HIGH);                    // Led ON

  // Create a (very) simple response, just to demonstrate
  String msg;
  msg = "ESP-Now Gateway \n";
  for (int i=1; i<=UNITS; i++) {
    String str2 = (i<10)?("0"+String(i)):(String(i));   // Make i to 2 pos string
    msg += String("Unit: "+str2+"  Temp: "+String(sensorData[i].temp)+" C\n");
  }
  server.send(200, "text/plain", msg );
  Serial.println ("Web server call:" + msg);
  
  digitalWrite (LED_BUILTIN, !LOW);                     // Led OFF}
}

void handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i = 0; i < server.args(); i++) {
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  server.send(404, "text/plain", message);
}

// ------------------------------------------------------------------------------------
void setup()
// ------------------------------------------------------------------------------------
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite (LED_BUILTIN, !HIGH);              // Led ON

  // Init Serial
  Serial.begin(115200);
  while (!Serial) {};
  delay(100);                                     // Needed for some boards
  Serial.println("\n\n");
  
  // Print sketch intro ---------------------------
  Serial.println();
  Serial.println("===========================================");
  Serial.println(SKETCH_NAME);
  Serial.println(SKETCH_VERSION);
  Serial.println(SKETCH_ABOUT);
  Serial.println("===========================================");

  // Connect to WiFi ------------------------------
  Serial.print("Connecting to WiFi ");

  WiFi.disconnect();                              // clear all configs
  WiFi.softAPdisconnect();

  // Set device in AP mode to begin with
  WiFi.mode(WIFI_AP_STA);                         // AP _and_ STA is required (!IMPORTANT)

  wifiMulti.addAP(SOFTAP_SSID, SOFTAP_PASS);      // I use wifiMulti ... just by habit, i guess ....
  while (wifiMulti.run() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  // Come here - we are connected
  Serial.println(" Done");

  // Print WiFi data
  Serial.println("Set as AP_STA station.");
  Serial.print  ("SSID: "); Serial.println(WiFi.SSID());
  Serial.print  ("Channel: "); Serial.println(WiFi.channel());
  Serial.print("IP address: "); Serial.println(WiFi.localIP());
  delay(1000);


  // Initialize ESP-Now ---------------------------

  // set SSID and channel 
  int channel = CHANNEL;
  if (WiFi.softAP(SOFTAP_SSID, SOFTAP_PASS, channel, 1)) {
    Serial.println("AP Config Success. AP SSID: " + String(SOFTAP_SSID));
  } else {
    Serial.println("AP Config failed.");
  }
  
  // Print MAC addresses
  Serial.print("AP MAC: "); Serial.println(WiFi.softAPmacAddress());
  Serial.print("STA MAC: "); Serial.println(WiFi.macAddress());

  // Init ESP-Now 
  if (esp_now_init() == 0) {
    Serial.println("ESP - Now Init Success");
  } else {
    Serial.println("ESP - Now Init Failed");
    ESP.restart();                                // just restart if we cant init ESP-Now
  }
  
  // ESP-Now is now initialized. Register a callback fcn for when data is received
  esp_now_register_recv_cb(OnDataRecv);

  // Set web server callback functions
  server.on("/", handleRoot);
  server.onNotFound(handleNotFound);

  // Start web server
  server.begin();
  Serial.print("WEB server started on SSID: "); Serial.print (WiFi.SSID()); Serial.print (" with IP: "); Serial.println(WiFi.localIP());
  
  digitalWrite (LED_BUILTIN, !LOW);               // Led OFF
}


// ------------------------------------------------------------------------------------
void loop()
// ------------------------------------------------------------------------------------
{
  server.handleClient();
}
