#include <Arduino.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <packets.pb.h>
#include <pb_arduino.h>
#include <pb_decode.h>

String version = "v0.0.1";

void setup() {
    SerialUSB.begin(115200);
    // IMPORTANT: Remove for production
    while (!SerialUSB)
        ;

    SerialUSB.println("[INFO] Initializing ground station TT&C subsystem (" + version +
                      ").");

    LoRa.enableCrc();
    LoRa.setPins(12, -1, 6);
    if (!LoRa.begin(915E6)) {
        SerialUSB.println("[ERROR] Couldn't start LoRa radio.");
        while (1)
            ;
    } else {
        SerialUSB.println("[INFO] LoRa radio started successfully.");
    }
}

void loop() {
    // Try to parse packet
    int packetSize = LoRa.parsePacket();
    if (packetSize) {
        // Initiate packet parsing
        SerialUSB.println("[INFO} Received telemetry packet.");

        // Save a buffer for the packet. TODO: Check buffer safety
        byte packet[128];
        size_t buffer_size;
        {
            // Use the Stream API to load the packet into a buffer.
            buffer_size = LoRa.readBytes(packet, packetSize);
            // Initialize the TelemetryPacket object.
            TelemetryPacket content = TelemetryPacket_init_zero;
            //  Push the buffer into a stream.
            pb_istream_t packet_stream = pb_istream_from_buffer(packet, buffer_size);
            // Decode the buffer according to our protobuf schema.
            bool decoding_status =
                pb_decode(&packet_stream, TelemetryPacket_fields, &content);
            // Error checking
            if (!decoding_status) {
                SerialUSB.print("[Error] Couldn't decode telemetry packet: ");
                SerialUSB.println(PB_GET_ERROR(&packet_stream));
            }
            // Pending more data, ignition data!
            SerialUSB.print("Received ignition: ");
            SerialUSB.println(content.ignition);
            SerialUSB.print("Received parachute_open: ");
            SerialUSB.println(content.parachute_open);
            SerialUSB.print("Received payload_separated: ");
            SerialUSB.println(content.payload_separated);
        }
        // print RSSI of packet. TODO: Add SNR and other signal metrics
        SerialUSB.print("[INFO] RSSI: ");
        SerialUSB.print(LoRa.packetRssi());
        SerialUSB.println(".");
    }
}
