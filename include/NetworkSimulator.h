#pragma once

#include "Broker.h"
#include "Device.h"
#include "Visualization.h"
#include <string>
#include <vector>
#include <memory>
#include <chrono>

// Forward declaration
struct GLFWwindow;

// Configuration constants
namespace {
    // Window settings
    constexpr int DEFAULT_WINDOW_WIDTH = 1280;
    constexpr int DEFAULT_WINDOW_HEIGHT = 800;
    constexpr char DEFAULT_WINDOW_TITLE[] = "MQTT 5.0 Network Simulator";
    constexpr float DEFAULT_CLEAR_COLOR[] = { 0.1f, 0.1f, 0.1f, 1.0f };

    // Device settings
    constexpr int DEFAULT_TELEMETRY_INTERVAL_MS = 1000;
}

/**
 * @brief NetworkSimulator class
 * Manages the MQTT network simulation and visualization
 */
class NetworkSimulator {
public:
    /**
     * @brief Construct a new NetworkSimulator object
     */
    NetworkSimulator();

    /**
     * @brief Destroy the NetworkSimulator object
     * Cleans up GLFW and ImGui resources
     */
    ~NetworkSimulator();

    // Delete copy and move constructors/operators
    NetworkSimulator(const NetworkSimulator&) = delete;
    NetworkSimulator& operator=(const NetworkSimulator&) = delete;
    NetworkSimulator(NetworkSimulator&&) = delete;
    NetworkSimulator& operator=(NetworkSimulator&&) = delete;

    /**
     * @brief Add a device to the network
     *
     * @param device_id Device identifier
     * @param telemetry_interval Interval for telemetry generation
     * @return std::shared_ptr<mqtt::Device> The created device
     */
    std::shared_ptr<mqtt::Device> addDevice(
        const std::string& device_id,
        std::chrono::milliseconds telemetry_interval =
        std::chrono::milliseconds(DEFAULT_TELEMETRY_INTERVAL_MS)
    );

    /**
     * @brief Initialize the simulator
     *
     * Sets up GLFW, ImGui, and creates initial UI components.
     * This must be called before run().
     *
     * @throws std::runtime_error If initialization fails
     */
    void initialize();

    /**
     * @brief Run the simulation
     *
     * Starts the main event loop and handles rendering.
     * initialize() must be called before this method.
     */
    void run();

private:
    /**
     * @brief Set up UI components
     *
     * Creates and configures all UI components used in the simulation.
     */
    void setupUIComponents();

    /**
     * @brief Set up initial devices for the simulation
     *
     * Creates a standard set of devices for demonstration.
     */
    void setupInitialDevices();

    /**
     * @brief Render the ImGui interface
     *
     * Renders all UI components and simulator interface.
     */
    void renderImGui();

    /**
     * @brief Initialize GLFW and ImGui
     *
     * @return bool True if initialization succeeded
     */
    bool initializeGlfwAndImGui();

    /**
     * @brief Clean up GLFW and ImGui resources
     */
    void cleanupGlfwAndImGui();

private:
    // Core simulation components
    std::shared_ptr<mqtt::Broker> broker;
    std::vector<std::shared_ptr<mqtt::Device>> devices;

    // UI components
    std::vector<std::unique_ptr<visualization::UIComponent>> ui_components;

    // GLFW/ImGui resources
    GLFWwindow* window = nullptr;
    bool initialized = false;
};