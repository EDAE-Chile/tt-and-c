#include <Arduino.h>
#include <LoRa.h>
#include <packets.pb.h>
#include <pb_arduino.h>
#include <pb_encode.h>

String version = "v0.0.2";

void setup() {
    SerialUSB.begin(115200);
    delay(1000);
    if (!SerialUSB) {
        delay(4000);
    }

    SerialUSB.println("[INFO] Initializing rocket TT&C subsystem (" + version + ").");

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
    // Buffer for the telemetry packet
    byte packet[512];
    size_t packet_length;

    {
        // Allocate space on the stack to store the packet data
        TelemetryPacket content = TelemetryPacket_init_zero;
        // Create an output stream based on the buffer
        pb_ostream_t packet_stream = pb_ostream_from_buffer(packet, sizeof(packet));

        // Fill packet with test data
        content.parachute_open = false;
        content.payload_separated = false;
        content.gps_reporting_status = TelemetryPacket_GPSStatus_OPERATIONAL;

        content.has_latitude = 1;
        content.latitude = 69.3329392;
        content.has_longitude = 1;
        content.longitude = -70.6075899;

        content.mobile_status = false;
        content.has_signal_strength = true;
        content.signal_strength = -20.334f;

        content.imu_status = true;
        content.barometer_status = true;
        content.thermometer_status = true;

        // content.imu_data = TelemetryPacket_IMU_init_zero;
        content.has_imu_data = true;
        content.imu_data.acceleration_x = 0.125f;
        content.imu_data.acceleration_y = -0.223f;
        content.imu_data.acceleration_z = 12.33422f;

        content.imu_data.gyroscope_x = 3.34423f;
        content.imu_data.gyroscope_y = -3.4523f;
        content.imu_data.gyroscope_z = -9.4523f;

        content.imu_data.magnetometer_x = 5.3523f;
        content.imu_data.magnetometer_y = 12.3523f;
        content.imu_data.magnetometer_z = 0.3523f;

        content.imu_data.secondary_temperature = -2.3523f;

        content.has_air_pressure = true;
        content.air_pressure = 234.2321;
        content.has_computed_height = true;
        content.computed_height = 230.3f;

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

    delay(4000);
}
