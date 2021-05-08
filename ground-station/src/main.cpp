#include <Arduino.h>
#include <LoRa.h>
#include <packets.pb.h>
#include <pb_arduino.h>
#include <pb_decode.h>

String version = "v0.0.2";

void setup() {
    SerialUSB.begin(115200);
    while (!SerialUSB)
        ;

    SerialUSB.println("[INFO] Initializing ground station TT&C subsystem (" + version +
                      ").");

    LoRa.setTxPower(20);
    LoRa.setSpreadingFactor(11);
    LoRa.setSignalBandwidth(500E3);
    LoRa.setCodingRate4(5);
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
        byte packet[512];
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
            SerialUSB.print("[DATA] Received parachute_open: ");
            SerialUSB.println(content.parachute_open);
            SerialUSB.print("[DATA] Received payload_separated: ");
            SerialUSB.println(content.payload_separated);

            SerialUSB.print("[DATA] Received gps_reporting_status: ");
            SerialUSB.println(content.gps_reporting_status);

            SerialUSB.print("[DATA] Received latitude: ");
            SerialUSB.println(content.latitude);

            SerialUSB.print("[DATA] Received longitude: ");
            SerialUSB.println(content.longitude);

            SerialUSB.print("[DATA] Received mobile_status: ");
            SerialUSB.println(content.mobile_status);

            SerialUSB.print("[DATA] Received signal_strength: ");
            SerialUSB.println(content.signal_strength);

            SerialUSB.print("[DATA] Received imu_status: ");
            SerialUSB.println(content.imu_status);

            SerialUSB.print("[DATA] Received barometer_status: ");
            SerialUSB.println(content.barometer_status);

            SerialUSB.print("[DATA] Received gps_reporting_status: ");
            SerialUSB.println(content.thermometer_status);

            SerialUSB.print("[DATA] Received secondary_temperature: ");
            SerialUSB.println(content.imu_data.secondary_temperature);

            SerialUSB.print("[DATA] Received air_pressure: ");
            SerialUSB.println(content.air_pressure);

            SerialUSB.print("[DATA] Received computed_height: ");
            SerialUSB.println(content.computed_height);
        }
        // print RSSI of packet. TODO: Add SNR and other signal metrics
        SerialUSB.print("[INFO] RSSI: ");
        SerialUSB.print(LoRa.packetRssi());
        SerialUSB.print(", SNR: ");
        SerialUSB.print(LoRa.packetSnr());
        SerialUSB.print(", PFR: ");
        SerialUSB.print(LoRa.packetFrequencyError());
        SerialUSB.println(".");
    }
}
