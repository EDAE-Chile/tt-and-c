#include <Arduino.h>
#include <LoRa.h>
#include <RH_RF95.h>
#include <SPI.h>
#include <packets.pb.h>
#include <pb_arduino.h>
#include <pb_encode.h>

String version = "v0.0.1";

void setup() {
    SerialUSB.begin(115200);
    // IMPORTANT: Remove for production
    while (!SerialUSB)
        ;

    SerialUSB.println("[INFO] Initializing rocket TT&C subsystem (" + version + ").");

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
    // Buffer for the telemetry packet
    byte packet[128];
    size_t packet_length;

    {
        // Allocate space on the stack to store the packet data
        TelemetryPacket content = TelemetryPacket_init_zero;
        // Create an output stream based on the buffer
        pb_ostream_t packet_stream = pb_ostream_from_buffer(packet, sizeof(packet));

        // Fill packet with test data
        content.ignition = true;
        content.parachute_open = false;
        content.payload_separated = false;

        // Encode the packet according to our protobuf schema.
        bool encoding_status =
            pb_encode(&packet_stream, TelemetryPacket_fields, &content);
        packet_length = packet_stream.bytes_written;

        // Error checkng
        if (!encoding_status) {
            SerialUSB.print("[Error] Couldn't encode telemetry packet: ");
            SerialUSB.println(PB_GET_ERROR(&packet_stream));
        }
    }

    SerialUSB.print("[INFO] Sending telemetry packet, length ");
    SerialUSB.print(packet_length);
    SerialUSB.println(".");

    // Send the packet through LoRa
    bool radio_status = LoRa.beginPacket();
    if (radio_status) {
        int write_size = LoRa.write(packet, packet_length);
        bool packet_transmission_status = LoRa.endPacket();
        if (packet_transmission_status && write_size > 0) {
            SerialUSB.print("[INFO] Telemetry packet sent, size ");
            SerialUSB.print(write_size);
            SerialUSB.println(".");
        } else {
            SerialUSB.println("[ERROR] Failure while completing packet transmission.");
        }
    } else {
        SerialUSB.println(
            "[ERROR]: Radio failed or was busy while trying to send telemetry packet.");
    }

    delay(10000);
}
