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

/**
 * @brief NetworkSimulator class
 */
class NetworkSimulator {
public:
    /**
     * @brief Construct a new NetworkSimulator object
     */
    NetworkSimulator();

    /**
     * @brief Destroy the NetworkSimulator object
     */
    ~NetworkSimulator();

    // Delete copy and move constructors/operators
    NetworkSimulator(const NetworkSimulator&) = delete;
    NetworkSimulator& operator=(const NetworkSimulator&) = delete;
    NetworkSimulator(NetworkSimulator&&) = delete;
    NetworkSimulator& operator=(NetworkSimulator&&) = delete;

    /**
     * @brief Add a device to the network
     */
    void addDevice(const std::string& device_id,
        std::chrono::milliseconds telemetry_interval = std::chrono::milliseconds(1000));

    /**
     * @brief Initialize the simulator
     */
    void initialize();

    /**
     * @brief Run the simulation
     */
    void run();

    /**
     * @brief Render the ImGui interface
     */
    void renderImGui();

private:
    /**
     * @brief Initialize GLFW and ImGui
     */
    bool initializeGlfwAndImGui();

    /**
     * @brief Clean up GLFW and ImGui
     */
    void cleanupGlfwAndImGui();

private:
    std::shared_ptr<mqtt::Broker> broker;
    std::vector<std::shared_ptr<mqtt::Device>> devices;

    // UI components
    std::vector<std::unique_ptr<visualization::UIComponent>> ui_components;

    // GLFW/ImGui
    GLFWwindow* window = nullptr;
    bool initialized = false;
};