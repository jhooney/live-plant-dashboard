/*
 * Smart device monitoring plant health
 * Running on XIAO-ESP32-C3
 * Talking to Monk Makes Plant Monitor
 * Reporting data over WiFi to Raspberry Pi via MQTT
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>  // Include the ArduinoJson library
#include <secrets.h>

#define RX_PIN D7 //in
#define TX_PIN D6 //out
#define DEVICE_ID 1

const char* ssid = WIFI_SSID;        // Replace with your network SSID
const char* password = WIFI_PASS;  // Replace with your network password
const char* mqtt_server = ROBINSERVER_IP;  // Replace with your Raspberry Pi IP address

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long previousMillis = 0;   // Stores the last time 'j' was sent
const long interval = 60000;        // Interval at which to send 'j' (60 seconds)

void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("XIAOESP32C3Client")) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Initialize Serial Monitor
  Serial.begin(115200);
  while (!Serial) {
    ; // Wait for the serial port to connect. Needed for native USB port only.
  }

  setup_wifi();
  client.setServer(mqtt_server, 1883);

  // Initialize Serial1 (UART) for RS232 communication
  Serial1.begin(9600, SERIAL_8N1, RX_PIN, TX_PIN);
  Serial.println("Serial communication with RS232 device started.");
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  unsigned long currentMillis = millis();

  // Check if it's time to send 'j'
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    Serial1.print('j');
    Serial.println("Sent 'j' to RS232 device.");
  }

  // Read data from the RS232 device
  if (Serial1.available()) {
    String dataFromRS232 = Serial1.readString();
    Serial.print("Received from RS232:");
    Serial.println(dataFromRS232);
  
    // Parse the received JSON
    StaticJsonDocument<200> doc;  // Adjust the size as needed
    DeserializationError error = deserializeJson(doc, dataFromRS232);

    if (!error) {
      // Add the device_id field
      doc["device_id"] = DEVICE_ID;

      // Convert the updated JSON back to a string
      String modifiedJson;
      serializeJson(doc, modifiedJson);

      Serial.print("Modified JSON: ");
      Serial.println(modifiedJson);

      // Publish the modified JSON over MQTT
      client.publish("plant_monitor/data", modifiedJson.c_str());
      Serial.println("Published JSON to MQTT.");
//don't know why this isn't working, leaving for the time being as I'm pressed for time
//    } else if (dataFromRS232.equals("OK")){
//      Serial.print("Turning LED off");
//      Serial1.print("l");
    } else {
      Serial.println("Turning LED off");
      Serial1.print("l");
      Serial.print("Failed to parse JSON:");
      Serial.print(dataFromRS232);
    }
  }

  // If data is available on the Serial Monitor, send it to the RS232 device
  if (Serial.available()) {
    String dataToRS232 = Serial.readString();
    Serial1.print(dataToRS232);
    Serial.print("Sent to RS232: ");
    Serial.println(dataToRS232);
  }
}
