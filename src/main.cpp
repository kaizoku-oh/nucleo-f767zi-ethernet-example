#include <Arduino.h>
#include <STM32Ethernet.h>
#include <PubSubClient.h>

static void mqtt_callback(char *topic, byte *payload, unsigned int length);
static void mqtt_reconnect(void);

// Static IP address to use in case the DHCP client fails to get an address
IPAddress staticIP(192, 168, 1, 55);
EthernetClient ethernetClient;
PubSubClient mqttClient(ethernetClient);

const char *mqttServerHostname = "test.mosquitto.org";
const uint16_t mqttServerPort = 1883;

void setup() {
  Serial.begin(115200);
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
  mqttClient.setServer(mqttServerHostname, mqttServerPort);
  mqttClient.setCallback(mqtt_callback);
}

void loop() {
  if(!mqttClient.connected()) {
    mqtt_reconnect();
  }
  mqttClient.loop();
}

static void mqtt_callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for(int i=0; i<length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();
}

static void mqtt_reconnect(void) {
  // Loop until we're reconnected
  while(!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if(mqttClient.connect("nucleo-f767zi-client")) {
      Serial.println("Connected");
      // Once connected, publish an announcement...
      mqttClient.publish("nucleo/test", "hello world");
      // ... and resubscribe
      mqttClient.subscribe("nucleo/test");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again to connect in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
