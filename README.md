# MQTT 5.0 Network Simulator

A visual simulator for MQTT 5.0 IoT networks with real-time messaging visualization using Dear ImGui.

## Overview

This application simulates an MQTT 5.0 network with a central broker and multiple client devices. It provides a graphical interface to visualize message flows between the broker and connected devices in real-time, making it ideal for learning about MQTT protocols, testing publish/subscribe patterns, and demonstrating IoT communication concepts.

## Features

- **MQTT 5.0 Protocol Support**: Implements key MQTT 5.0 features including enhanced message properties, improved subscription options, and more
- **Real-time Visualization**: Dynamic graphical representation of message flows between broker and devices
- **Simulated Devices**: Auto-generates realistic telemetry data from simulated IoT devices
- **Interactive Command Center**: Send custom commands to devices through the broker
- **Message History**: Track and inspect all messages for each device
- **Topic Wildcards**: Support for single-level (+) and multi-level (#) wildcards
- **Custom QoS Levels**: Configure Quality of Service for messages
- **Retained Messages**: Full support for retained message delivery

## Building the Project

### Prerequisites

- Visual Studio 2022
- C++17 compatible compiler
- OpenGL support
- GLFW 3.3+ library

### Build Instructions

1. Clone this repository
2. Open the solution file in Visual Studio 2022
3. Make sure the ThirdParty directory contains required libraries:
   - Dear ImGui (in ThirdParty/imgui)
   - GLFW (in ThirdParty/glfw)
4. Build the solution (F7)
5. Run the application (F5)

### Project Structure

```
MQTTSimulator/
├── Include/                   # Header files
│   ├── Message.h              # MQTT Message class
│   ├── QoS.h                  # Quality of Service enum
│   ├── Device.h               # MQTT Client Device class
│   ├── Broker.h               # MQTT Broker class
│   ├── NetworkSimulator.h     # Network Simulator class
│   └── Visualization.h        # UI components
├── Source/                    # Implementation files
│   ├── Message.cpp            # Message implementation
│   ├── Device.cpp             # Device implementation
│   ├── Broker.cpp             # Broker implementation
│   ├── NetworkSimulator.cpp   # NetworkSimulator implementation
│   ├── Visualization.cpp      # Visualization implementation
│   └── main.cpp               # Application entry point
└── ThirdParty/                # External libraries
    ├── imgui/                 # Dear ImGui library
    └── glfw/                  # GLFW library
```

## Using the Simulator

1. **Network Overview**: View broker status, device count, and message statistics
2. **Message Flow**: Visualize real-time messaging between broker and devices
3. **Device Details**: View device-specific information including subscriptions and message history
4. **Command Center**: Send commands to specific devices or broadcast to all devices

### Adding Devices

Click the "Add Device" button in the Network Overview panel to add new devices to the simulation.

### Sending Commands

1. Enter a topic (e.g., "command/device_1" or "command/all")
2. Type your payload
3. Select QoS level
4. Toggle "Retained" if needed
5. Click "Send Command"

## Extending the Simulator

The object-oriented design makes it easy to extend the simulator:

- **Custom Device Types**: Inherit from the Device class to create specialized device types
- **Advanced Visualization**: Add new visualization components by implementing the UIComponent interface
- **Real Broker Connection**: Modify the Broker class to connect to an actual MQTT broker
- **Network Conditions**: Add network condition simulation (latency, packet loss)

## Still to do:
 - **Additional Unit Testing**: Need to expand unit testing. Diagnose and fix errors with VS2022 project file linking to test project
 - **Add custom JSON packet definitions**: Allow for packet definitions to be specified by an input JSON file. Allow for flexibility in schema definitions.
 - **Pan/Zoom on network diagram**: Current diagram is fixed - need to add common pan/zoom controls
 - **Mouseover Status**: Show current status of node on mouseover
 - **Message Viewer**: Show live message content
 - **Visual changes to nodes upon command**: Node color or shape should change on specific sent commands.

## Acknowledgments

- [Dear ImGui](https://github.com/ocornut/imgui) for the immediate-mode GUI
- [GLFW](https://www.glfw.org/) for OpenGL window management
