#pragma once

#include <cstddef>
#include <chrono>

namespace mqtt {
    namespace constants {

        //-------------------------------------------------------------------------
        // Message history settings
        //-------------------------------------------------------------------------

        // Maximum number of messages to keep in broker history
        constexpr size_t BROKER_MESSAGE_HISTORY_SIZE = 100;

        // Maximum number of messages to keep in device history
        constexpr size_t DEVICE_MESSAGE_HISTORY_SIZE = 50;

        // Maximum number of messages to display in visualization
        constexpr size_t MAX_DISPLAYED_MESSAGES = 20;

        //-------------------------------------------------------------------------
        // Thread timing constants
        //-------------------------------------------------------------------------

        // Interval between message processing cycles in broker
        constexpr int MESSAGE_PROCESSING_INTERVAL_MS = 10;

        // Random variation range for telemetry timing
        constexpr int TELEMETRY_RANDOM_MIN_MS = 100;
        constexpr int TELEMETRY_RANDOM_MAX_MS = 500;

        // Default telemetry generation interval
        constexpr int DEFAULT_TELEMETRY_INTERVAL_MS = 1000;

        //-------------------------------------------------------------------------
        // Telemetry simulation constants
        //-------------------------------------------------------------------------

        // Temperature range (Celsius)
        constexpr double TEMPERATURE_MIN = 15.0;
        constexpr double TEMPERATURE_MAX = 30.0;

        // Humidity range (Percentage)
        constexpr double HUMIDITY_MIN = 30.0;
        constexpr double HUMIDITY_MAX = 80.0;

        // Pressure range (hPa)
        constexpr double PRESSURE_MIN = 990.0;
        constexpr double PRESSURE_MAX = 1020.0;

        // Battery voltage range (V)
        constexpr double BATTERY_MIN = 3.0;
        constexpr double BATTERY_MAX = 4.2;

        //-------------------------------------------------------------------------
        // Topic constants
        //-------------------------------------------------------------------------

        // Command topic prefix
        constexpr char COMMAND_TOPIC_PREFIX[] = "command/";

        // All devices command topic
        constexpr char ALL_DEVICES_TOPIC[] = "command/all";

        // Telemetry topic prefix
        constexpr char TELEMETRY_TOPIC_PREFIX[] = "telemetry/";

        //-------------------------------------------------------------------------
        // Default device names
        //-------------------------------------------------------------------------

        constexpr char DEFAULT_TEMP_SENSOR_ID[] = "sensor_temp";
        constexpr char DEFAULT_HUMIDITY_SENSOR_ID[] = "sensor_humidity";
        constexpr char DEFAULT_VALVE_ACTUATOR_ID[] = "actuator_valve";
        constexpr char DEFAULT_GATEWAY_ID[] = "gateway";

        //-------------------------------------------------------------------------
        // Device intervals (milliseconds)
        //-------------------------------------------------------------------------

        constexpr int TEMP_SENSOR_INTERVAL_MS = 2000;
        constexpr int HUMIDITY_SENSOR_INTERVAL_MS = 3000;
        constexpr int VALVE_ACTUATOR_INTERVAL_MS = 5000;
        constexpr int GATEWAY_INTERVAL_MS = 1000;

    } // namespace constants
} // namespace mqtt