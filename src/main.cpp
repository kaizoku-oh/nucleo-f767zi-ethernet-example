#include <Arduino.h>
#include <STM32Ethernet.h>

// Static IP address to use in case the DHCP client fails to get an address
IPAddress staticIP(192, 168, 1, 55);

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
}

void loop() {
}
