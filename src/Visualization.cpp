#include "Visualization.h"
#include "Broker.h"
#include "Device.h"
#include "Message.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <ctime>
#include <unordered_set>
#include <unordered_map>

#define _USE_MATH_DEFINES
#include <math.h>

namespace visualization {

    // MessageFlow implementation
    MessageFlow::MessageFlow(std::shared_ptr<mqtt::Broker> broker,
        const std::vector<std::shared_ptr<mqtt::Device>>& devices)
        : broker(broker), devices(devices) {
    }

    void MessageFlow::render() {
        // Set up the canvas for visualization
        ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, style::MESSAGE_CANVAS_HEIGHT);
        ImGui::BeginChild("MessageCanvas", canvas_size, true, ImGuiWindowFlags_HorizontalScrollbar);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

        // Calculate center position, draw broker in center
        ImVec2 center_pos(canvas_pos.x + canvas_size.x / 2, canvas_pos.y + 70);
        drawBroker(draw_list, center_pos);

        // Cache device positions
        std::map<std::string, ImVec2> device_positions = calculateDevicePositions(center_pos);

        // Draw devices
        for (const auto& device : devices) {
            const auto& device_id = device->getId();
            drawDevice(draw_list, device_id, device_positions[device_id]);
        }

        // Get messages, display
        const auto& all_messages = broker->getMessageHistory();
        size_t msg_count = all_messages.size();
        size_t start_idx = (msg_count > style::MAX_VISIBLE_MESSAGES) ?
            (msg_count - style::MAX_VISIBLE_MESSAGES) : 0;

        // Draw messages
        for (size_t i = start_idx; i < msg_count; i++) {
            const auto& msg = all_messages[i];
            float alpha = 0.2f + 0.8f * ((i - start_idx) / static_cast<float>(msg_count - start_idx));

            // Start and end positions
            bool is_publish = !msg.getSenderId().empty();
            ImVec2 start_pos = getPosition(msg.getSenderId(), device_positions, center_pos);
            ImVec2 end_pos = getPosition(msg.getTargetId(), device_positions, center_pos);

            drawMessage(draw_list, msg, start_pos, end_pos, alpha);
        }

        ImGui::EndChild();
    }

    void MessageFlow::drawBroker(ImDrawList* draw_list, ImVec2 position) {
        draw_list->AddCircleFilled(position, style::BROKER_RADIUS, style::BROKER_COLOR);

        // Add text centered on the broker
        const char* label = "BROKER";
        ImVec2 text_size = ImGui::CalcTextSize(label);
        draw_list->AddText(
            ImVec2(position.x - text_size.x / 2, position.y - text_size.y / 2),
            style::TEXT_COLOR,
            label
        );
    }

    void MessageFlow::drawDevice(ImDrawList* draw_list, const std::string& id, ImVec2 position) {
		// Draw circle represnting the device, and label
        draw_list->AddCircleFilled(position, style::DEVICE_RADIUS, style::DEVICE_COLOR);
        std::string label = id;
        if (label.length() > 8) {
            label = label.substr(0, 6) + "..";
        }

        // Center text on device
        ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
        draw_list->AddText(
            ImVec2(position.x - text_size.x / 2, position.y - text_size.y / 2),
            style::TEXT_COLOR,
            label.c_str()
        );
    }

    void MessageFlow::drawMessage(ImDrawList* draw_list, const mqtt::Message& msg,
        ImVec2 start, ImVec2 end, float alpha) {
		// Is public or suscribe message?
        bool is_publish = !msg.getSenderId().empty();

        // Calculate color with alpha
        ImU32 color = is_publish ?
            IM_COL32(ImColor(style::PUBLISH_COLOR).Value.x * 255,
                ImColor(style::PUBLISH_COLOR).Value.y * 255,
                ImColor(style::PUBLISH_COLOR).Value.z * 255,
                255 * alpha) :
            IM_COL32(ImColor(style::SUBSCRIBE_COLOR).Value.x * 255,
                ImColor(style::SUBSCRIBE_COLOR).Value.y * 255,
                ImColor(style::SUBSCRIBE_COLOR).Value.z * 255,
                255 * alpha);

        // Draw connection line
        draw_list->AddLine(start, end, color, style::LINE_THICKNESS);

        // Draw direction arrow
        drawMessageArrow(draw_list, start, end, color,
            is_publish ? style::BROKER_RADIUS : style::DEVICE_RADIUS,
            is_publish);

        // Draw topic
        ImVec2 mid_point((start.x + end.x) / 2, (start.y + end.y) / 2);
        drawMessageTopic(draw_list, msg.getTopic(), mid_point, alpha);
    }

    void MessageFlow::drawMessageArrow(ImDrawList* draw_list, ImVec2 start, ImVec2 end,
        ImU32 color, float radius, bool is_publish) {
        // Calculate direction vector, normalize
        ImVec2 dir(end.x - start.x, end.y - start.y);
        float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
        dir.x /= dist;
        dir.y /= dist;

		// Find arrow position, normal vector perpendicular to direction
        ImVec2 arrow_pos(
            start.x + dir.x * (dist - radius - 10),
            start.y + dir.y * (dist - radius - 10)
        );
        ImVec2 norm(-dir.y, dir.x);

        // Draw arrowhead triangle
        draw_list->AddTriangleFilled(
            ImVec2(arrow_pos.x, arrow_pos.y),
            ImVec2(arrow_pos.x - dir.x * style::ARROW_SIZE + norm.x * style::ARROW_SIZE,
                arrow_pos.y - dir.y * style::ARROW_SIZE + norm.y * style::ARROW_SIZE),
            ImVec2(arrow_pos.x - dir.x * style::ARROW_SIZE - norm.x * style::ARROW_SIZE,
                arrow_pos.y - dir.y * style::ARROW_SIZE - norm.y * style::ARROW_SIZE),
            color
        );
    }

    void MessageFlow::drawMessageTopic(ImDrawList* draw_list, const std::string& topic,
        ImVec2 position, float alpha) {
        // Truncate topic if too long
        std::string display_topic = topic;
        if (display_topic.length() > 15) {
            display_topic = display_topic.substr(0, 12) + "...";
        }

        // Calculate text size for background
        ImVec2 text_size = ImGui::CalcTextSize(display_topic.c_str());

        // Draw background rectangle
        ImU32 bg_color = IM_COL32(
            ImColor(style::BACKGROUND_COLOR).Value.x * 255,
            ImColor(style::BACKGROUND_COLOR).Value.y * 255,
            ImColor(style::BACKGROUND_COLOR).Value.z * 255,
            ImColor(style::BACKGROUND_COLOR).Value.w * 255 * alpha
        );

        draw_list->AddRectFilled(
            ImVec2(position.x - text_size.x / 2 - 3, position.y - text_size.y / 2 - 1),
            ImVec2(position.x + text_size.x / 2 + 3, position.y + text_size.y / 2 + 1),
            bg_color
        );

        // Draw text
        ImU32 text_color = IM_COL32(255, 255, 255, 255 * alpha);
        draw_list->AddText(
            ImVec2(position.x - text_size.x / 2, position.y - text_size.y / 2),
            text_color,
            display_topic.c_str()
        );
    }

    std::map<std::string, ImVec2> MessageFlow::calculateDevicePositions(ImVec2 center) {
        std::map<std::string, ImVec2> positions;
        int num_devices = static_cast<int>(devices.size());

        // Cases: no devices
        if (num_devices == 0) {
            return positions;
        }

        // find positions around circle
        for (int i = 0; i < num_devices; i++) {
            float angle = static_cast<float>(2 * M_PI * i / num_devices);
            float x = center.x + style::RING_RADIUS * cos(angle);
            float y = center.y + style::RING_RADIUS * sin(angle);

            positions[devices[i]->getId()] = ImVec2(x, y);
        }
        return positions;
    }
    // Return default position (broker) if ID is empty
    ImVec2 MessageFlow::getPosition(const std::string& id, const std::map<std::string, ImVec2>& positions, ImVec2 default_pos) {
        if (id.empty() || positions.find(id) == positions.end()) {
            return default_pos;
        }
        return positions.at(id);
    }

    // DeviceDetails implementation
    DeviceDetails::DeviceDetails(const std::vector<std::shared_ptr<mqtt::Device>>& devices)
        : devices(devices) {
    }

    void DeviceDetails::render() {
        // Search input filter
        ImGui::Text("Search:");
        ImGui::SameLine();
        if (ImGui::InputText("##DeviceSearch", search_buffer, IM_ARRAYSIZE(search_buffer))) {
            search_filter = search_buffer;
        }

        ImGui::Separator();

        // Select device with filter
        if (ImGui::BeginCombo("Select Device", selected_device >= 0 ?
            devices[selected_device]->getId().c_str() : "None")) {

            for (int i = 0; i < static_cast<int>(devices.size()); i++) {
                const auto& device_id = devices[i]->getId();

                // Apply filter
                if (!search_filter.empty() &&
                    device_id.find(search_filter) == std::string::npos) {
                    continue;
                }

                bool is_selected = (selected_device == i);
                if (ImGui::Selectable(device_id.c_str(), is_selected))
                    selected_device = i;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (selected_device >= 0 && selected_device < static_cast<int>(devices.size())) {
            renderDeviceInfo(devices[selected_device]);
        }
    }

    void DeviceDetails::renderDeviceInfo(const std::shared_ptr<mqtt::Device>& device) {
        ImGui::Text("Device ID: %s", device->getId().c_str());

        // Collapsing section of subscribed topics
        if (ImGui::CollapsingHeader("Subscriptions", ImGuiTreeNodeFlags_DefaultOpen)) {
            renderSubscriptions(device);
        }

        // Show message history
        if (ImGui::CollapsingHeader("Message History", ImGuiTreeNodeFlags_DefaultOpen)) {
            renderMessageHistory(device);
        }
    }

    void DeviceDetails::renderSubscriptions(const std::shared_ptr<mqtt::Device>& device) {
        const auto& topics = device->getSubscribedTopics();

        if (topics.empty()) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No active subscriptions");
            return;
        }

		// Show each topic with bullet points
        for (const auto& topic : topics) {
            ImGui::BulletText("%s", topic.c_str());

            // Tooltip on hover
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Topic: %s", topic.c_str());
                ImGui::Text("Wildcards: %s",
                    (topic.find('+') != std::string::npos || topic.find('#') != std::string::npos) ?
                    "Yes" : "No");
                ImGui::EndTooltip();
            }
        }
    }

    void DeviceDetails::renderMessageHistory(const std::shared_ptr<mqtt::Device>& device) {
        const auto& messages = device->getMessageHistory();

        // Filter controls
        static bool show_incoming = true;
        static bool show_outgoing = true;
        static char topic_filter[128] = "";

        ImGui::Checkbox("Incoming", &show_incoming);
        ImGui::SameLine();
        ImGui::Checkbox("Outgoing", &show_outgoing);
        ImGui::SameLine();
        ImGui::SetNextItemWidth(150);
        ImGui::InputText("Topic Filter", topic_filter, IM_ARRAYSIZE(topic_filter));

        // Create scrollable area for messages
        ImGui::BeginChild("DeviceMessages", ImVec2(0, 200), true);

        // Count visible messages
        int visible_count = 0;

        for (const auto& msg : messages) {
            // See if incoming or outgoing
            bool is_incoming = !msg.getTargetId().empty();

            // Apply filters
            if ((is_incoming && !show_incoming) || (!is_incoming && !show_outgoing)) {
                continue;
            }

            // Apply topic filter
            if (topic_filter[0] != '\0' &&
                msg.getTopic().find(topic_filter) == std::string::npos) {
                continue;
            }

            // Count this message
            visible_count++;

            // Set color based on message type
            ImGui::PushStyleColor(ImGuiCol_Text,
                is_incoming ? ImVec4(0.2f, 0.7f, 0.2f, 1.0f) :
                ImVec4(0.7f, 0.2f, 0.2f, 1.0f));

            // Format timestamp
            auto time_point = std::chrono::system_clock::to_time_t(msg.getTimestamp());
            std::tm tm_info;
#ifdef _MSC_VER
            localtime_s(&tm_info, &time_point); // Safe version for Visual Studio
#else
            * (&tm_info) = *std::localtime(&time_point);
#endif
            char time_str[9];
            std::strftime(time_str, sizeof(time_str), "%H:%M:%S", &tm_info);

            // Display message with timestamp
            ImGui::Text("[%s] %s %s: %s",
                time_str,
                is_incoming ? "RECV" : "SEND",
                msg.getTopic().c_str(),
                msg.getPayload().substr(0, 30).c_str());

            // Detailed tooltip when hovering
            if (ImGui::IsItemHovered()) {
                ImGui::BeginTooltip();
                ImGui::Text("Topic: %s", msg.getTopic().c_str());
                ImGui::Text("QoS: %d", static_cast<int>(msg.getQoS()));
                ImGui::Text("Retained: %s", msg.isRetained() ? "Yes" : "No");
                ImGui::Text("Full Payload: %s", msg.getPayload().c_str());
                ImGui::EndTooltip();
            }

            ImGui::PopStyleColor();
        }

		// Show message if none visible
        if (visible_count == 0) {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f),
                "No messages match the current filters");
        }

        ImGui::EndChild();
    }

    // CommandCenter implementation
    CommandCenter::CommandCenter(std::shared_ptr<mqtt::Broker> broker,
        const std::vector<std::shared_ptr<mqtt::Device>>& devices)
        : broker(broker), devices(devices) {
    }

    void CommandCenter::render() {
        // Right-click suggestions for topic input
        ImGui::InputText("Topic", command_topic, IM_ARRAYSIZE(command_topic));

        // Show topic suggestions on right-click
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup("TopicSuggestions");
        }

        // Draw suggestions popup
        showTopicSuggestions();

        // Payload input
        ImGui::InputTextMultiline("Payload", command_payload, IM_ARRAYSIZE(command_payload),
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3));

        // QoS 
        const char* qos_items[] = {
            "QoS 0 (At most once)",
            "QoS 1 (At least once)",
            "QoS 2 (Exactly once)"
        };
        ImGui::Combo("QoS", &command_qos, qos_items, IM_ARRAYSIZE(qos_items));

        // Retained flag
        ImGui::Checkbox("Retained", &command_retained);

        // Send button
        if (ImGui::Button("Send Command", ImVec2(120, 0))) {
            sendCommand();
        }

        showCommandStatus();
    }

    void CommandCenter::showTopicSuggestions() {
        if (ImGui::BeginPopup("TopicSuggestions")) {
            ImGui::Text("Common Topics:");

            // Device-specific commands
            if (ImGui::BeginMenu("Device Commands")) {
                for (const auto& device : devices) {
                    std::string suggestion = mqtt::constants::COMMAND_TOPIC_PREFIX + device->getId();
                    if (ImGui::MenuItem(suggestion.c_str())) {
                        strcpy_s(command_topic, suggestion.c_str());
                    }
                }
                ImGui::EndMenu();
            }

            // Command patterns
            if (ImGui::BeginMenu("Common Patterns")) {
                if (ImGui::MenuItem("All Devices")) {
                    strcpy_s(command_topic, mqtt::constants::ALL_DEVICES_TOPIC);
                }
                if (ImGui::MenuItem("Group Commands")) {
                    strcpy_s(command_topic, "command/group/+");
                }
                if (ImGui::MenuItem("Wildcard Example")) {
                    strcpy_s(command_topic, "sensors/#");
                }
                ImGui::EndMenu();
            }

            ImGui::EndPopup();
        }
    }

    void CommandCenter::sendCommand() {
        // Create a message with current settings
        mqtt::Message message(command_topic, command_payload,
            static_cast<mqtt::QoS>(command_qos), command_retained);
        message.setSenderId("command_center");

        // Add timestamp
        message.setContentType("text/plain");

        // Publish message through broker
        if (broker) {
            broker->publish(message);
            command_sent = true;
            command_sent_time = static_cast<float>(ImGui::GetTime());
        }
    }

    void CommandCenter::showCommandStatus() {
        if (command_sent) {
            float elapsed = static_cast<float>(ImGui::GetTime()) - command_sent_time;
            if (elapsed < style::MESSAGE_FADE_DURATION) {
                ImGui::SameLine();
                float alpha = 1.0f - (elapsed / style::MESSAGE_FADE_DURATION);
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, alpha), "Command sent!");
            }
            else {
                command_sent = false;
            }
        }
    }

    // NetworkOverview implementation
    NetworkOverview::NetworkOverview(std::shared_ptr<mqtt::Broker> broker,
        const std::vector<std::shared_ptr<mqtt::Device>>& devices,
        std::function<void()> add_device_callback)
        : broker(broker),
        devices(devices),
        add_device_callback(add_device_callback) {
    }

    void NetworkOverview::render() {
        // Broker information section
        renderBrokerInfo();

        // Device controls section
        renderDeviceControls();

        ImGui::Separator();

        // Statistics section
        renderStatistics();
    }

    void NetworkOverview::renderBrokerInfo() {
        // Broker status & information
        bool is_active = broker->getMessageHistory().size() > 0;

        ImGui::Text("Broker: ");
        ImGui::SameLine();
        ImGui::TextColored(
            is_active ? ImVec4(0.0f, 1.0f, 0.0f, 1.0f) : ImVec4(1.0f, 0.5f, 0.0f, 1.0f),
            is_active ? "Active" : "Idle"
        );

        ImGui::Text("Broker ID: %s", broker->getId().c_str());
    }

    void NetworkOverview::renderDeviceControls() {
        // Device count, "add" button
        ImGui::Text("Devices: %zu", devices.size());

        if (ImGui::Button("Add Device")) {
            add_device_callback();
        }

        // Show device type selector - future implementation
        ImGui::SameLine();
        static int device_type = 0;
        const char* device_types[] = { "Generic Device", "Sensor", "Actuator", "Gateway" };
        ImGui::SetNextItemWidth(150);
        ImGui::Combo("##DeviceType", &device_type, device_types, IM_ARRAYSIZE(device_types));

        // table of connected devices
        if (!devices.empty() && ImGui::BeginTable("DevicesTable", 3,
            ImGuiTableFlags_Borders |
            ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Device ID");
            ImGui::TableSetupColumn("Type");
            ImGui::TableSetupColumn("Status");
            ImGui::TableHeadersRow();

            for (const auto& device : devices) {
                ImGui::TableNextRow();

                // Device ID
                ImGui::TableSetColumnIndex(0);
                ImGui::Text("%s", device->getId().c_str());

                // Device type (TODO: placeholder for future implementation)
                ImGui::TableSetColumnIndex(1);
                if (device->getId().find("sensor") != std::string::npos) {
                    ImGui::Text("Sensor");
                }
                else if (device->getId().find("actuator") != std::string::npos) {
                    ImGui::Text("Actuator");
                }
                else if (device->getId().find("gateway") != std::string::npos) {
                    ImGui::Text("Gateway");
                }
                else {
                    ImGui::Text("Generic");
                }

                // Device status (based on history)
                ImGui::TableSetColumnIndex(2);
                if (device->getMessageHistory().empty()) {
                    ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Inactive");
                }
                else {
                    auto last_msg_time = device->getMessageHistory().back().getTimestamp();
                    auto now = std::chrono::system_clock::now();
                    auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - last_msg_time).count();

                    if (elapsed < 5) {
                        ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f), "Active");
                    }
                    else if (elapsed < 30) {
                        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "Idle (%llds)", elapsed);
                    }
                    else {
                        ImGui::TextColored(ImVec4(1.0f, 0.5f, 0.0f, 1.0f), "Timeout (%llds)", elapsed);
                    }
                }
            }

            ImGui::EndTable();
        }
    }

    void NetworkOverview::renderStatistics() {
        size_t total_pub = 0;
        size_t total_sub = 0;
        float message_rate = 0.0f;

        // Calculate and display statistics
        calculateStatistics(total_pub, total_sub, message_rate);

        ImGui::Text("Statistics:");
        ImGui::Indent();

        ImGui::Text("Publish Messages: %zu", total_pub);
        ImGui::Text("Subscribe Deliveries: %zu", total_sub);

        if (message_rate > 0.0f) {
            ImGui::Text("Messaging Rate: %.1f msg/sec", message_rate);

            // Show histogram of message rates
            static float rate_history[60] = { 0 };
            static int rate_history_offset = 0;

            // Update rate history
            rate_history[rate_history_offset] = message_rate;
            rate_history_offset = (rate_history_offset + 1) % IM_ARRAYSIZE(rate_history);

            // Calculate min/max for scaling
            float rate_min = rate_history[0], rate_max = rate_history[0];
            for (int i = 1; i < IM_ARRAYSIZE(rate_history); i++) {
                rate_min = std::min(rate_min, rate_history[i]);
                rate_max = std::max(rate_max, rate_history[i]);
            }

            // Ensure valid range
            if (rate_max == rate_min) rate_max = rate_min + 1.0f;

            // Plot histogram
            ImGui::PlotLines("##RateHistory", rate_history, IM_ARRAYSIZE(rate_history),
                rate_history_offset, "msg/sec", rate_min, rate_max, ImVec2(200, 50));
        }
        else {
            ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "Insufficient data for rate calculation");
        }

        ImGui::Unindent();

        // Topic distribution
        ImGui::Text("Topic Distribution:");
        ImGui::Indent();

        std::unordered_map<std::string, int> topic_counts;

        // Count messages/topic
        for (const auto& msg : broker->getMessageHistory()) {
            topic_counts[msg.getTopic()]++;
        }

        // Sort by message count
        std::vector<std::pair<std::string, int>> sorted_topics(topic_counts.begin(), topic_counts.end());
        std::sort(sorted_topics.begin(), sorted_topics.end(),
            [](const auto& a, const auto& b) { return a.second > b.second; });

        // Show top 5 topics
        int max_topics = std::min(5, static_cast<int>(sorted_topics.size()));
        for (int i = 0; i < max_topics; i++) {
            const auto& topic_data = sorted_topics[i];
            float percentage = 100.0f * topic_data.second / broker->getMessageHistory().size();

            ImGui::Text("%s: %d msgs (%.1f%%)", topic_data.first.c_str(), topic_data.second, percentage);

            // Bar graph
            ImGui::SameLine(250);
            ImGui::ProgressBar(percentage / 100.0f, ImVec2(100, 8), "");
        }

        ImGui::Unindent();
    }

    void NetworkOverview::calculateStatistics(size_t& total_pub, size_t& total_sub, float& message_rate) {
        total_pub = 0;
        total_sub = 0;
        message_rate = 0.0f;

        const auto& messages = broker->getMessageHistory();

        // Count message types
        for (const auto& msg : messages) {
            if (msg.getTargetId().empty()) {
                total_pub++;
            }
            else {
                total_sub++;
            }
        }

        // Calculate message rate
        if (!messages.empty() && messages.size() > 1) {
            auto oldest = messages.front().getTimestamp();
            auto newest = messages.back().getTimestamp();

            auto duration = std::chrono::duration_cast<std::chrono::seconds>(newest - oldest).count();
            if (duration > 0) {
                message_rate = messages.size() / static_cast<float>(duration);
            }
        }
    }

} // namespace visualization