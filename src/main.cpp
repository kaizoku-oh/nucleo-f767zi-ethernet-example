#include <Arduino.h>
#include <STM32Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

// MQTT callback function called when a publish message is received
static void mqtt_callback(char *topic, byte *payload, unsigned int length);
// Helper function used to reconnect to MQTT server when connection is lost
static void mqtt_reconnect(void);

// Static IP address to use in case the DHCP client fails to get an address
const IPAddress staticIP(192, 168, 1, 55);
// MQTT configs
const uint16_t mqttServerPort = 1883;
const char *mqttServerHostname = "test.mosquitto.org";
const char *mqttSubscribeTopic = "gdg/test";
// Bool used to track the LED state
bool ledIsOn;
// TCP client object to be used by the MQTT client
EthernetClient ethernetClient;
// MQTT client object
PubSubClient mqttClient(ethernetClient);
// Static JSON document object to parse the received message over MQTT
StaticJsonDocument<200> jsonDoc;

void setup() {
  // Initialize the LED
  ledIsOn = false;
  pinMode(LED_BLUE, OUTPUT);
  // Initialize the serial port
  Serial.begin(115200);
  // Setup ethernet with DHCP else use a static IP
  if(Ethernet.begin() == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    Serial.print("Configuring Ethernet using static IP: ");
    Serial.println(staticIP);
    Ethernet.begin(staticIP);
  }
  else {
    Serial.println("Ethernet is configured successfully using DHCP");
  }
  Serial.print("Connected to network. IP = ");
  Serial.println(Ethernet.localIP());
  // Setup MQTT configs
  mqttClient.setServer(mqttServerHostname, mqttServerPort);
  mqttClient.setCallback(mqtt_callback);
}

void loop() {
  // Check if client is disconnected, if so reconnect
  if(!mqttClient.connected()) {
    mqtt_reconnect();
  }
  // Process incoming MQTT packets
  mqttClient.loop();
}

static void mqtt_callback(char *topic, byte *payload, unsigned int length) {
  DeserializationError error;

  // Print the received message and topic
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for(int i=0; i<length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
  // Parse the received JSON message and take action accordingly
  error = deserializeJson(jsonDoc, payload);
  if(0 == error) {
    Serial.println("JSON message parsed successfully");
    if(jsonDoc["lights_state"] == "on") {
      // Turn on LED
      digitalWrite(LED_BLUE, HIGH);
      ledIsOn = true;
    }
    else if(jsonDoc["lights_state"] == "off") {
      // Turn off LED
      digitalWrite(LED_BLUE, LOW);
      ledIsOn = false;
    }
    else if(jsonDoc["lights_state"] == "toggle") {
      // Toggle LED and update its state
      digitalWrite(LED_BLUE, (true == ledIsOn) ? LOW : HIGH);
      ledIsOn = (true == ledIsOn) ? false : true;
    } else {
      Serial.println("Unknown json param");
    }
  } else {
    Serial.println("Failed to parse JSON message");
  }
}

static void mqtt_reconnect(void) {
  // Loop until we're reconnected
  while(!mqttClient.connected()) {
    Serial.println("Attempting MQTT connection...");
    // Attempt to connect
    if(mqttClient.connect("nucleo-f767zi-client")) {
      Serial.print("Connected to ");
      Serial.println(mqttServerHostname);
      // Once connected, subscribe
      if(mqttClient.subscribe(mqttSubscribeTopic)) {
        Serial.print("Subscribed to \"");
        Serial.print(mqttSubscribeTopic);
        Serial.println("\"");
      } else {
        Serial.println("Failed to subscribe");
      }
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" trying to connect again in 5 seconds...");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
