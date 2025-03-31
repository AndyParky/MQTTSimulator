#include "NetworkSimulator.h"
#include <iostream>
#include <stdexcept>
#include "Constants.h"

/**
 * @brief Application entry point
 *
 * @return int Exit code
 */
int main(int argc, char** argv) {
    try {
        // Create the network simulator
        NetworkSimulator simulator;

        // Add some initial devices
        simulator.addDevice(mqtt::constants::DEFAULT_TEMP_SENSOR_ID, std::chrono::milliseconds(mqtt::constants::TEMP_SENSOR_INTERVAL_MS));
        simulator.addDevice(mqtt::constants::DEFAULT_HUMIDITY_SENSOR_ID, std::chrono::milliseconds(mqtt::constants::HUMIDITY_SENSOR_INTERVAL_MS));
        simulator.addDevice(mqtt::constants::DEFAULT_VALVE_ACTUATOR_ID, std::chrono::milliseconds(mqtt::constants::VALVE_ACTUATOR_INTERVAL_MS));
        simulator.addDevice(mqtt::constants::DEFAULT_GATEWAY_ID, std::chrono::milliseconds(mqtt::constants::GATEWAY_INTERVAL_MS));

        // Initialize and run the simulator
        simulator.initialize();
        simulator.run();

        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}