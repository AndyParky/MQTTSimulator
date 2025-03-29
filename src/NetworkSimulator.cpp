#include "../include/NetworkSimulator.h"
#include "../include/Broker.h"
#include "../include/Device.h"
#include "../include/Visualization.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <functional>

NetworkSimulator::NetworkSimulator()
    : broker(std::make_shared<mqtt::Broker>("main_broker")) {
}

NetworkSimulator::~NetworkSimulator() {
    cleanupGlfwAndImGui();
}

void NetworkSimulator::addDevice(const std::string& device_id,
    std::chrono::milliseconds telemetry_interval) {
    devices.push_back(std::make_shared<mqtt::Device>(device_id, broker, telemetry_interval));

    // Subscribe to device-specific command topics
    devices.back()->subscribe("command/" + device_id);
    devices.back()->subscribe("command/all");

    // Add message handler to display received commands
    devices.back()->addMessageHandler([device_id](const mqtt::Message& msg) {
        std::cout << "Device " << device_id << " received command: "
            << msg.getTopic() << " -> " << msg.getPayload() << std::endl;
        });
}

void NetworkSimulator::initialize() {
    if (initialized) {
        return;
    }

    // Initialize GLFW and ImGui
    if (!initializeGlfwAndImGui()) {
        throw std::runtime_error("Failed to initialize GLFW and ImGui");
    }

    // Create UI components
    auto add_device_callback = [this]() {
        std::string id = "device_" + std::to_string(devices.size() + 1);
        addDevice(id);
        };

    ui_components.push_back(std::make_unique<visualization::NetworkOverview>(
        broker, devices, add_device_callback));
    ui_components.push_back(std::make_unique<visualization::MessageFlow>(broker, devices));
    ui_components.push_back(std::make_unique<visualization::DeviceDetails>(devices));
    ui_components.push_back(std::make_unique<visualization::CommandCenter>(broker, devices));

    initialized = true;
}

void NetworkSimulator::run() {
    if (!initialized) {
        initialize();
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the simulator UI
        renderImGui();

        // Rendering
        ImGui::Render();
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }

    cleanupGlfwAndImGui();
}

void NetworkSimulator::renderImGui() {
    // Main window
    ImGui::Begin("MQTT 5.0 Network Simulator");

    // Render UI components in collapsing headers
    const char* headers[] = {
        "Network Overview",
        "Message Flow",
        "Device Details",
        "Command Center"
    };

    for (size_t i = 0; i < ui_components.size(); i++) {
        if (ImGui::CollapsingHeader(headers[i], ImGuiTreeNodeFlags_DefaultOpen)) {
            ui_components[i]->render();
        }
    }

    ImGui::End();
}

bool NetworkSimulator::initializeGlfwAndImGui() {
    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    window = glfwCreateWindow(1280, 800, "MQTT 5.0 Network Simulator", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);

    return true;
}

void NetworkSimulator::cleanupGlfwAndImGui() {
    if (window) {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        glfwDestroyWindow(window);
        glfwTerminate();

        window = nullptr;
    }
}