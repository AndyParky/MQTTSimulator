#include "NetworkSimulator.h"
#include "Broker.h"
#include "Device.h"
#include "Visualization.h"
#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <functional>

//-------------------------------------------------------------------------
// Constructor / Destructor
//-------------------------------------------------------------------------

NetworkSimulator::NetworkSimulator()
    : broker(std::make_shared<mqtt::Broker>("main_broker")) {
}

NetworkSimulator::~NetworkSimulator() {
    cleanupGlfwAndImGui();
}

//-------------------------------------------------------------------------
// Device Management
//-------------------------------------------------------------------------

std::shared_ptr<mqtt::Device> NetworkSimulator::addDevice(
    const std::string& device_id,
    std::chrono::milliseconds telemetry_interval) {

    // Create the device
    auto device = std::make_shared<mqtt::Device>(device_id, broker, telemetry_interval);
    devices.push_back(device);

    // Subscribe to device-specific command topics
    device->subscribe("command/" + device_id);
    device->subscribe("command/all");

    // Add message handler to display received commands
    device->addMessageHandler([device_id](const mqtt::Message& msg) {
        std::cout << "Device " << device_id << " received command: "
            << msg.getTopic() << " -> " << msg.getPayload() << std::endl;
        });

    return device;
}

void NetworkSimulator::setupInitialDevices() {
    // Add a set of standard devices for demonstration
    addDevice("sensor_temp", std::chrono::milliseconds(mqtt::constants::TEMP_SENSOR_INTERVAL_MS));
    addDevice("sensor_humidity", std::chrono::milliseconds(mqtt::constants::HUMIDITY_SENSOR_INTERVAL_MS));
    addDevice("actuator_valve", std::chrono::milliseconds(mqtt::constants::VALVE_ACTUATOR_INTERVAL_MS));
    addDevice("gateway", std::chrono::milliseconds(mqtt::constants::GATEWAY_INTERVAL_MS));
}

//-------------------------------------------------------------------------
// Lifecycle Management
//-------------------------------------------------------------------------

void NetworkSimulator::initialize() {
    if (initialized) {
        return;
    }

    // Initialize GLFW and ImGui
    if (!initializeGlfwAndImGui()) {
        throw std::runtime_error("Failed to initialize GLFW and ImGui");
    }

    // Create initial devices if none exist
    if (devices.empty()) {
        setupInitialDevices();
    }

    // Setup UI components
    setupUIComponents();

    initialized = true;
}

void NetworkSimulator::run() {
    if (!initialized) {
        initialize();
    }

    // Main loop
    while (!glfwWindowShouldClose(window)) {
        // Process events
        glfwPollEvents();

        // Start the ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // Render the simulator UI
        renderImGui();

        // Finalize rendering
        ImGui::Render();

        // Update framebuffer
        int display_w, display_h;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(
            DEFAULT_CLEAR_COLOR[0],
            DEFAULT_CLEAR_COLOR[1],
            DEFAULT_CLEAR_COLOR[2],
            DEFAULT_CLEAR_COLOR[3]
        );
        glClear(GL_COLOR_BUFFER_BIT);

        // Render ImGui
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        // Swap buffers
        glfwSwapBuffers(window);
    }

    cleanupGlfwAndImGui();
}

//-------------------------------------------------------------------------
// UI Rendering
//-------------------------------------------------------------------------

void NetworkSimulator::setupUIComponents() {
    // Clear any existing components
    ui_components.clear();

    // Create device addition callback
    auto add_device_callback = [this]() {
        std::string id = "device_" + std::to_string(devices.size() + 1);
        addDevice(id);
        };

    // Create UI components
    ui_components.push_back(std::make_unique<visualization::NetworkOverview>(
        broker, devices, add_device_callback));
    ui_components.push_back(std::make_unique<visualization::MessageFlow>(broker, devices));
    ui_components.push_back(std::make_unique<visualization::DeviceDetails>(devices));
    ui_components.push_back(std::make_unique<visualization::CommandCenter>(broker, devices));
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

    // Ensure we don't try to render more components than we have headers
    const size_t component_count = std::min(ui_components.size(), sizeof(headers) / sizeof(headers[0]));

    // Render each component in its own header section
    for (size_t i = 0; i < component_count; i++) {
        if (ImGui::CollapsingHeader(headers[i], ImGuiTreeNodeFlags_DefaultOpen)) {
            ui_components[i]->render();
        }
    }

    ImGui::End();
}

//-------------------------------------------------------------------------
// GLFW/ImGui Handling
//-------------------------------------------------------------------------

bool NetworkSimulator::initializeGlfwAndImGui() {
    // Set error callback
    glfwSetErrorCallback([](int error, const char* description) {
        std::cerr << "GLFW Error " << error << ": " << description << std::endl;
        });

    // Initialize GLFW
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return false;
    }

    // Configure OpenGL context
    const char* glsl_version = "#version 130";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    // Create window
    window = glfwCreateWindow(
        mqtt::constants::DEFAULT_WINDOW_WIDTH,
        mqtt::constants::DEFAULT_WINDOW_HEIGHT,
        DEFAULT_WINDOW_TITLE,
        nullptr,
        nullptr
    );

    if (!window) {
        const char* error_description;
        glfwGetError(&error_description);
        std::cerr << "Failed to create GLFW window: " <<
            (error_description ? error_description : "Unknown error") << std::endl;
        glfwTerminate();
        return false;
    }

    // Configure context
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync

    // Setup ImGui
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();

    // Configure ImGui style
    ImGui::StyleColorsDark();

    // Initialize ImGui backends
    if (!ImGui_ImplGlfw_InitForOpenGL(window, true)) {
        std::cerr << "Failed to initialize ImGui GLFW backend" << std::endl;
        return false;
    }

    if (!ImGui_ImplOpenGL3_Init(glsl_version)) {
        std::cerr << "Failed to initialize ImGui OpenGL backend" << std::endl;
        return false;
    }

    return true;
}

void NetworkSimulator::cleanupGlfwAndImGui() {
    if (window) {
        // Cleanup ImGui resources
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();

        // Cleanup GLFW resources
        glfwDestroyWindow(window);
        glfwTerminate();

        window = nullptr;
    }
}