#pragma once

#include "imgui.h"
#include "Constants.h"
#include <vector>
#include <memory>
#include <functional>
#include <map>
#include <string>

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
     * @brief Common styling constants for visualization
     */
    namespace style {
        // Colors
        static constexpr ImU32 BROKER_COLOR = IM_COL32(80, 150, 200, 255);
        static constexpr ImU32 DEVICE_COLOR = IM_COL32(100, 220, 120, 255);
        static constexpr ImU32 PUBLISH_COLOR = IM_COL32(240, 100, 100, 255);
        static constexpr ImU32 SUBSCRIBE_COLOR = IM_COL32(100, 100, 240, 255);
        static constexpr ImU32 TEXT_COLOR = IM_COL32(255, 255, 255, 255);
        static constexpr ImU32 BACKGROUND_COLOR = IM_COL32(50, 50, 50, 200);

        // Sizes
        static constexpr float BROKER_RADIUS = 40.0f;
        static constexpr float DEVICE_RADIUS = 25.0f;
        static constexpr float ARROW_SIZE = 6.0f;
        static constexpr float LINE_THICKNESS = 2.0f;

        // Layout
        static constexpr float RING_RADIUS = 200.0f;
        static constexpr float MESSAGE_CANVAS_HEIGHT = 400.0f;

        // Animation
        static constexpr float MESSAGE_FADE_DURATION = 2.0f;
        static constexpr size_t MAX_VISIBLE_MESSAGES = 20;
    }

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
        void drawBroker(ImDrawList* draw_list, ImVec2 position);
        void drawDevice(ImDrawList* draw_list, const std::string& id, ImVec2 position);
        void drawMessage(ImDrawList* draw_list, const mqtt::Message& msg,
            ImVec2 start, ImVec2 end, float alpha);
        void drawMessageArrow(ImDrawList* draw_list, ImVec2 start, ImVec2 end,
            ImU32 color, float radius, bool is_publish);
        void drawMessageTopic(ImDrawList* draw_list, const std::string& topic,
            ImVec2 position, float alpha);

        std::map<std::string, ImVec2> calculateDevicePositions(ImVec2 center);
        ImVec2 getPosition(const std::string& id, const std::map<std::string, ImVec2>& positions, ImVec2 default_pos);

        std::shared_ptr<mqtt::Broker> broker;
        std::vector<std::shared_ptr<mqtt::Device>> devices;
        bool positions_need_update = true;
    };

    /**
     * @brief DeviceDetails visualization component
     */
    class DeviceDetails : public UIComponent {
    public:
        explicit DeviceDetails(const std::vector<std::shared_ptr<mqtt::Device>>& devices);
        void render() override;

    private:
        void renderDeviceInfo(const std::shared_ptr<mqtt::Device>& device);
        void renderSubscriptions(const std::shared_ptr<mqtt::Device>& device);
        void renderMessageHistory(const std::shared_ptr<mqtt::Device>& device);

        const std::vector<std::shared_ptr<mqtt::Device>>& devices;
        int selected_device = -1;
        std::string search_filter;
        char search_buffer[128] = "";
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
        void showTopicSuggestions();
        void sendCommand();
        void showCommandStatus();

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
        void renderBrokerInfo();
        void renderDeviceControls();
        void renderStatistics();
        void calculateStatistics(size_t& total_pub, size_t& total_sub, float& message_rate);

        std::shared_ptr<mqtt::Broker> broker;
        const std::vector<std::shared_ptr<mqtt::Device>>& devices;
        std::function<void()> add_device_callback;
    };

} // namespace visualization