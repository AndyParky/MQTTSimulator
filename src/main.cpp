#include "../Include/NetworkSimulator.h"
#include <iostream>
#include <stdexcept>

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
        simulator.addDevice("sensor_temp", std::chrono::milliseconds(2000));
        simulator.addDevice("sensor_humidity", std::chrono::milliseconds(3000));
        simulator.addDevice("actuator_valve", std::chrono::milliseconds(5000));
        simulator.addDevice("gateway", std::chrono::milliseconds(1000));

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