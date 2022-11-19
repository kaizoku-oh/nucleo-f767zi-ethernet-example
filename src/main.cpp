#include <Arduino.h>
#include <STM32Ethernet.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static void mqtt_callback(char *topic, byte *payload, unsigned int length);
static void mqtt_reconnect(void);

// Static IP address to use in case the DHCP client fails to get an address
IPAddress staticIP(192, 168, 1, 55);
EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);
StaticJsonDocument<200> jsonDoc;
bool ledIsOn;

const uint16_t mqttServerPort = 1883;
const char *mqttServerHostname = "test.mosquitto.org";
const char *mqttSubscribeTopic = "gdg/test";

void setup() {
  ledIsOn = false;
  pinMode(LED_BLUE, OUTPUT);
  Serial.begin(115200);
  // Ethernet setup
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
  // MQTT setup
  Serial.println(Ethernet.localIP());
  mqttClient.setServer(mqttServerHostname, mqttServerPort);
  mqttClient.setCallback(mqtt_callback);
}

void loop() {
  if(!mqttClient.connected()) {
    mqtt_reconnect();
  }
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
      digitalWrite(LED_BLUE, HIGH);
      ledIsOn = true;
    }
    else if(jsonDoc["lights_state"] == "off") {
      digitalWrite(LED_BLUE, LOW);
      ledIsOn = false;
    }
    else if(jsonDoc["lights_state"] == "toggle") {
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
        Serial.print("Subscribed to ");
        Serial.println(mqttSubscribeTopic);
      } else {
        Serial.println("Failed to subscribe");
      }
    } else {
      Serial.print("Failed to connect, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again to connect in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
