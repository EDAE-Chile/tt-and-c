#include <Arduino.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <packets.pb.h>

int counter = 0;

void setup() {
  SerialUSB.begin(115200);
  while (!SerialUSB)
    ;

  SerialUSB.println("LoRa Sender");

  LoRa.setPins(12, -1, 6);
  if (!LoRa.begin(915E6)) {
    SerialUSB.println("Starting LoRa failed!");
    while (1)
      ;
  }
}

void loop() {
  SerialUSB.print("Sending packet ");
  SerialUSB.println(counter);

  // send packet
  LoRa.beginPacket();
  LoRa.print("hello ");
  LoRa.print(counter);
  LoRa.endPacket();

  counter++;

  delay(5000);
}
