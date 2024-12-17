/*
 * Smart device monitoring plant health
 * Running on XIAO-ESP32-C3
 * Talking to Monk Makes Plant Monitor
 * Reporting data over WiFi to Raspberry Pi via MQTT
 */

#include <WiFiMulti.h>
WiFiMulti wifiMulti;
#include <ArduinoJson.h>
#include <production-secrets.h>
//#include <testing-secrets.h>
#include <InfluxDbClient.h>
#include <InfluxDbCloud.h>

#define RX_PIN D7 //in
#define TX_PIN D6 //out
#define DEVICE_ID "4" //CHANGE PER DEVICE

#define WIRED_MODE false //cannot use Serial in battery mode on this board so need to disable

// Time zone info
#define TZ_INFO "UTC-5"

// Declare InfluxDB client instance with preconfigured InfluxCloud certificate
InfluxDBClient client(INFLUXDB_URL, INFLUXDB_ORG, INFLUXDB_BUCKET, INFLUXDB_TOKEN, InfluxDbCloud2CACert);

Point sensor("plant_monitors");

unsigned long previousMillis = 0;   // Stores the last time 'j' was sent
const long interval = 60000;        // Interval at which to send 'j' (60 seconds)
unsigned long timeSinceTimeReset = 0; // Stores last time reset time interval
const long resync_time_interval = 600000;        // Interval at which to send 'j' (10 minutes)

String influx_writing_values; //used to track influx db communication
bool influx_write_status;

void setup_wifi() {
  WiFi.mode(WIFI_STA);
  wifiMulti.addAP(WIFI_SSID, WIFI_PASS);

  if(WIRED_MODE){
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(WIFI_SSID);
  }
  
  while (wifiMulti.run() != WL_CONNECTED) {
    if(WIRED_MODE){
      Serial.print(".");
    }
    delay(100);
  }

  if(WIRED_MODE){
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
  }
}

void turn_led_off() {
    if(WIRED_MODE){Serial.println("Turning LED off");}
    Serial1.print("l");
}

void setup() {
  if(WIRED_MODE){
    // Initialize Serial Monitor
    Serial.begin(115200);
  }

  setup_wifi();
  
  // Accurate time is necessary for certificate validation and writing in batches
  // We use the NTP servers in your area as provided by: https://www.pool.ntp.org/zone/
  // Syncing progress and the time will be printed to Serial.
  timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov");

  if(WIRED_MODE){
    // Check server connection
    if (client.validateConnection()) {
      Serial.print("Connected to InfluxDB: ");
      Serial.println(client.getServerUrl());
    } else {
      Serial.print("InfluxDB connection failed: ");
      Serial.println(client.getLastErrorMessage());
    }
  }
    
  // Add tags to the data point
  sensor.addTag("device", DEVICE_ID);
  sensor.addTag("SSID", WiFi.SSID());

  // Initialize Serial1 (UART) for RS232 communication
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  if(WIRED_MODE){Serial.println("Serial communication with RS232 device started.");}
}

void loop() {
  // Ensure the InfluxDB client is still connected
  if (wifiMulti.run() != WL_CONNECTED) {
    setup_wifi(); // Reconnect to WiFi if disconnected
  }
  
  // Clear fields for reusing the point. Tags will remain the same as set above.
  sensor.clearFields();

  unsigned long currentMillis = millis();

  // Check if it's time to resync time value
  if (currentMillis - timeSinceTimeReset >= resync_time_interval) {
    if(WIRED_MODE){Serial.println("resyncing time value");}
    timeSync(TZ_INFO, "pool.ntp.org", "time.nis.gov"); //re-sync time
    timeSinceTimeReset = currentMillis;
  }
  
  // Check if it's time to send 'j'
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial1.print('j');
    if(WIRED_MODE){Serial.println("Sent 'j' to RS232 device.");}
  }

  // Read data from the RS232 device
  if (Serial1.available()) {
    String dataFromRS232 = Serial1.readString();
    if(WIRED_MODE){
      Serial.print("Received from RS232: ");
      Serial.println(dataFromRS232);
    }
  
    // Parse the received JSON
    StaticJsonDocument<200> doc;  // Adjust the size as needed
    DeserializationError error = deserializeJson(doc, dataFromRS232);

    turn_led_off();
    
    if (!error) {
      // Add the device_id field
      doc["device_id"] = DEVICE_ID;

      // Extract sensor data from the JSON
      float wetness = doc["wetness"];
      float humidity = doc["humidity"];
      float temp = doc["temp"];

      // Add tags and fields
      sensor.clearFields();
      sensor.addField("wetness", wetness);
      sensor.addField("humidity", humidity);
      sensor.addField("temperature", temp);
      sensor.addField("signal_strength", WiFi.RSSI());

      influx_writing_values = sensor.toLineProtocol();
      if(WIRED_MODE){
        // Print what are we exactly writing
        Serial.print("Writing: ");
        Serial.println(influx_writing_values);
      }

      influx_write_status = client.writePoint(sensor);

      if(WIRED_MODE){
        // Write the point to InfluxDB
        if (influx_write_status) {
          Serial.println("Data written to InfluxDB successfully.");
        } else {
          Serial.print("InfluxDB write failed: ");
          Serial.println(client.getLastErrorMessage());
        }
      }
    } else {
      if(WIRED_MODE){
        Serial.print("Failed to parse JSON: ");
        Serial.println(dataFromRS232);
      }
    }
  }

  if(WIRED_MODE){
    // If data is available on the Serial Monitor, send it to the RS232 device
    if (Serial.available()) {
      String dataToRS232 = Serial.readString();
      Serial1.print(dataToRS232);
      Serial.print("Sent to RS232: ");
      Serial.println(dataToRS232);
    }
  }
}
