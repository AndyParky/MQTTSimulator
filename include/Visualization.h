#pragma once

#include "imgui.h"
#include <vector>
#include <memory>
#include <functional>

// Forward declarations
namespace mqtt {
    class Message;
    class Device;
    class Broker;
}

/**
 * @brief Visualization namespace for UI components
 */
namespace visualization {

    /**
     * @brief Abstract UI component class
     */
    class UIComponent {
    public:
        virtual ~UIComponent() = default;
        virtual void render() = 0;
    };

    /**
     * @brief MessageFlow visualization component
     */
    class MessageFlow : public UIComponent {
    public:
        MessageFlow(std::shared_ptr<mqtt::Broker> broker,
            const std::vector<std::shared_ptr<mqtt::Device>>& devices);
        void render() override;

    private:
        std::shared_ptr<mqtt::Broker> broker;
        std::vector<std::shared_ptr<mqtt::Device>> devices;
    };

    /**
     * @brief DeviceDetails visualization component
     */
    class DeviceDetails : public UIComponent {
    public:
        explicit DeviceDetails(const std::vector<std::shared_ptr<mqtt::Device>>& devices);
        void render() override;

    private:
        const std::vector<std::shared_ptr<mqtt::Device>>& devices;
        int selected_device = -1;
    };

    /**
     * @brief CommandCenter visualization component
     */
    class CommandCenter : public UIComponent {
    public:
        CommandCenter(std::shared_ptr<mqtt::Broker> broker,
            const std::vector<std::shared_ptr<mqtt::Device>>& devices);
        void render() override;

    private:
        std::shared_ptr<mqtt::Broker> broker;
        const std::vector<std::shared_ptr<mqtt::Device>>& devices;
        char command_topic[128] = "command/device";
        char command_payload[256] = "SET_PARAMETER:value";
        int command_qos = 0;
        bool command_retained = false;
        bool command_sent = false;
        float command_sent_time = 0.0f;
    };

    /**
     * @brief NetworkOverview visualization component
     */
    class NetworkOverview : public UIComponent {
    public:
        NetworkOverview(std::shared_ptr<mqtt::Broker> broker,
            const std::vector<std::shared_ptr<mqtt::Device>>& devices,
            std::function<void()> add_device_callback);
        void render() override;

    private:
        std::shared_ptr<mqtt::Broker> broker;
        const std::vector<std::shared_ptr<mqtt::Device>>& devices;
        std::function<void()> add_device_callback;
    };

} // namespace visualization