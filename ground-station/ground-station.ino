#include <Arduino.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <packets.pb.h>

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  Serial.println("LoRa Receiver");

  LoRa.setPins(12, -1, 6);
  if (!LoRa.begin(915E6)) {
    Serial.println("Starting LoRa failed!");
    while (1)
      ;
  }
}

void loop() {
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}
