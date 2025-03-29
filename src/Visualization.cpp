#include "../Include/Visualization.h"
#include "../Include/Broker.h"
#include "../Include/Device.h"
#include "../Include/Message.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_opengl3.h"
#include <GLFW/glfw3.h>
#include <cmath>
#include <algorithm>
#include <ctime>

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
        ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 400);
        ImGui::BeginChild("MessageCanvas", canvas_size, true, ImGuiWindowFlags_HorizontalScrollbar);

        ImDrawList* draw_list = ImGui::GetWindowDrawList();
        ImVec2 canvas_pos = ImGui::GetCursorScreenPos();

        // Draw broker in the center
        float center_x = canvas_pos.x + canvas_size.x / 2;
        float center_y = canvas_pos.y + 70;
        float broker_radius = 40;

        draw_list->AddCircleFilled(ImVec2(center_x, center_y), broker_radius,
            IM_COL32(80, 150, 200, 255));
        draw_list->AddText(ImVec2(center_x - 25, center_y - 7),
            IM_COL32(255, 255, 255, 255), "BROKER");

        // Draw devices in a circle around the broker
        int num_devices = static_cast<int>(devices.size());
        float device_radius = 25;
        float ring_radius = 200;

        std::vector<ImVec2> device_positions;
        for (int i = 0; i < num_devices; i++) {
            float angle = 2 * M_PI * i / num_devices;
            float x = center_x + ring_radius * cos(angle);
            float y = center_y + ring_radius * sin(angle);

            device_positions.push_back(ImVec2(x, y));

            // Draw device
            draw_list->AddCircleFilled(ImVec2(x, y), device_radius,
                IM_COL32(100, 220, 120, 255));

            // Draw device ID
            std::string label = devices[i]->getId();
            if (label.length() > 8) {
                label = label.substr(0, 6) + "..";
            }

            ImVec2 text_size = ImGui::CalcTextSize(label.c_str());
            draw_list->AddText(ImVec2(x - text_size.x / 2, y - text_size.y / 2),
                IM_COL32(255, 255, 255, 255), label.c_str());
        }

        // Draw message flows (last 20 messages)
        auto messages = broker->getMessageHistory();
        if (messages.size() > 20) {
            messages = std::vector<mqtt::Message>(messages.end() - 20, messages.end());
        }

        for (size_t i = 0; i < messages.size(); i++) {
            const auto& msg = messages[i];

            // Find device positions for sender and target
            ImVec2 start_pos, end_pos;
            bool is_publish = false;

            // Find sender position
            if (msg.getSenderId().empty()) {
                // From broker to device
                start_pos = ImVec2(center_x, center_y);
            }
            else {
                // From device to broker
                is_publish = true;
                for (size_t j = 0; j < devices.size(); j++) {
                    if (devices[j]->getId() == msg.getSenderId()) {
                        start_pos = device_positions[j];
                        break;
                    }
                }
            }

            // Find target position
            if (msg.getTargetId().empty()) {
                // To broker
                end_pos = ImVec2(center_x, center_y);
            }
            else {
                // To specific device
                for (size_t j = 0; j < devices.size(); j++) {
                    if (devices[j]->getId() == msg.getTargetId()) {
                        end_pos = device_positions[j];
                        break;
                    }
                }
            }

            // Calculate animation progress (messages fade out over time)
            float alpha = 0.2f + 0.8f * (i / (float)messages.size());

            // Message color based on type (publish or subscribe)
            ImU32 color = is_publish ?
                IM_COL32(240, 100, 100, 255 * alpha) :
                IM_COL32(100, 100, 240, 255 * alpha);

            // Draw message line
            draw_list->AddLine(start_pos, end_pos, color, 2.0f);

            // Draw message direction arrow
            ImVec2 dir = ImVec2(end_pos.x - start_pos.x, end_pos.y - start_pos.y);
            float dist = sqrt(dir.x * dir.x + dir.y * dir.y);
            dir.x /= dist;
            dir.y /= dist;

            ImVec2 arrow_pos = ImVec2(
                start_pos.x + dir.x * (dist - (is_publish ? broker_radius : device_radius) - 10),
                start_pos.y + dir.y * (dist - (is_publish ? broker_radius : device_radius) - 10)
            );

            float arrow_size = 6.0f;
            ImVec2 norm = ImVec2(-dir.y, dir.x);

            draw_list->AddTriangleFilled(
                ImVec2(arrow_pos.x, arrow_pos.y),
                ImVec2(arrow_pos.x - dir.x * arrow_size + norm.x * arrow_size,
                    arrow_pos.y - dir.y * arrow_size + norm.y * arrow_size),
                ImVec2(arrow_pos.x - dir.x * arrow_size - norm.x * arrow_size,
                    arrow_pos.y - dir.y * arrow_size - norm.y * arrow_size),
                color
            );

            // Draw topic name at the middle of the line
            ImVec2 mid_point = ImVec2(
                (start_pos.x + end_pos.x) / 2,
                (start_pos.y + end_pos.y) / 2
            );

            // Shorten topic name if too long
            std::string topic = msg.getTopic();
            if (topic.length() > 15) {
                topic = topic.substr(0, 12) + "...";
            }

            ImVec2 text_size = ImGui::CalcTextSize(topic.c_str());
            draw_list->AddRectFilled(
                ImVec2(mid_point.x - text_size.x / 2 - 3, mid_point.y - text_size.y / 2 - 1),
                ImVec2(mid_point.x + text_size.x / 2 + 3, mid_point.y + text_size.y / 2 + 1),
                IM_COL32(50, 50, 50, 200 * alpha)
            );
            draw_list->AddText(
                ImVec2(mid_point.x - text_size.x / 2, mid_point.y - text_size.y / 2),
                IM_COL32(255, 255, 255, 255 * alpha),
                topic.c_str()
            );
        }

        ImGui::EndChild();
    }

    // DeviceDetails implementation
    DeviceDetails::DeviceDetails(const std::vector<std::shared_ptr<mqtt::Device>>& devices)
        : devices(devices) {
    }

    void DeviceDetails::render() {
        // Device selector
        if (ImGui::BeginCombo("Select Device", selected_device >= 0 ?
            devices[selected_device]->getId().c_str() : "None")) {
            for (int i = 0; i < static_cast<int>(devices.size()); i++) {
                bool is_selected = (selected_device == i);
                if (ImGui::Selectable(devices[i]->getId().c_str(), is_selected))
                    selected_device = i;
                if (is_selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndCombo();
        }

        if (selected_device >= 0 && selected_device < static_cast<int>(devices.size())) {
            auto& device = devices[selected_device];

            ImGui::Text("Device ID: %s", device->getId().c_str());

            // Show subscribed topics
            if (ImGui::CollapsingHeader("Subscriptions", ImGuiTreeNodeFlags_DefaultOpen)) {
                const auto& topics = device->getSubscribedTopics();
                if (topics.empty()) {
                    ImGui::TextColored(ImVec4(0.7f, 0.7f, 0.7f, 1.0f), "No active subscriptions");
                }
                else {
                    for (const auto& topic : topics) {
                        ImGui::BulletText("%s", topic.c_str());
                    }
                }
            }

            // Show message history for this device
            if (ImGui::CollapsingHeader("Message History", ImGuiTreeNodeFlags_DefaultOpen)) {
                const auto& messages = device->getMessageHistory();

                ImGui::BeginChild("DeviceMessages", ImVec2(0, 200), true);
                for (const auto& msg : messages) {
                    // Determine if this is incoming or outgoing
                    bool is_incoming = !msg.getTargetId().empty();

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

                    ImGui::Text("[%s] %s %s: %s",
                        time_str,
                        is_incoming ? "RECV" : "SEND",
                        msg.getTopic().c_str(),
                        msg.getPayload().substr(0, 30).c_str());

                    ImGui::PopStyleColor();
                }
                ImGui::EndChild();
            }
        }
    }

    // CommandCenter implementation
    CommandCenter::CommandCenter(std::shared_ptr<mqtt::Broker> broker,
        const std::vector<std::shared_ptr<mqtt::Device>>& devices)
        : broker(broker), devices(devices) {
    }

    void CommandCenter::render() {
        ImGui::InputText("Topic", command_topic, IM_ARRAYSIZE(command_topic));

        // Topic suggestions
        if (ImGui::IsItemHovered() && ImGui::IsMouseClicked(1)) {
            ImGui::OpenPopup("TopicSuggestions");
        }

        if (ImGui::BeginPopup("TopicSuggestions")) {
            ImGui::Text("Common Topics:");
            bool clicked = false;

            for (const auto& device : devices) {
                std::string suggestion = "command/" + device->getId();
                if (ImGui::Selectable(suggestion.c_str())) {
                    strcpy_s(command_topic, suggestion.c_str());
                    clicked = true;
                }
            }

            if (ImGui::Selectable("command/all")) {
                strcpy_s(command_topic, "command/all");
                clicked = true;
            }

            ImGui::EndPopup();
        }

        ImGui::InputTextMultiline("Payload", command_payload, IM_ARRAYSIZE(command_payload),
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3));

        const char* qos_items[] = { "QoS 0 (At most once)", "QoS 1 (At least once)", "QoS 2 (Exactly once)" };
        ImGui::Combo("QoS", &command_qos, qos_items, IM_ARRAYSIZE(qos_items));

        ImGui::Checkbox("Retained", &command_retained);

        if (ImGui::Button("Send Command", ImVec2(120, 0))) {
            // If we're using a real MQTT client, we'd send through that
            // For simulation, we'll create a message and inject it into the broker

            mqtt::Message message(command_topic, command_payload,
                static_cast<mqtt::QoS>(command_qos), command_retained);
            message.setSenderId("command_center");

            if (broker) {
                broker->publish(message);
                command_sent = true;
                command_sent_time = ImGui::GetTime();
            }
        }

        // Show success message for a short time
        if (command_sent) {
            float elapsed = ImGui::GetTime() - command_sent_time;
            if (elapsed < 2.0f) {
                ImGui::SameLine();
                ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 1.0f - elapsed / 2.0f),
                    "Command sent!");
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
        ImGui::Text("Broker: %s", broker->getMessageHistory().size() > 0 ? "Active" : "Idle");
        ImGui::Text("Broker ID: %s", broker->getId().c_str());
        ImGui::Text("Devices: %zu", devices.size());
        ImGui::Text("Messages Processed: %zu", broker->getMessageHistory().size());

        if (ImGui::Button("Add Device")) {
            add_device_callback();
        }

        ImGui::Separator();

        // Show basic statistics
        size_t total_pub = 0;
        size_t total_sub = 0;

        // Count message types
        for (const auto& msg : broker->getMessageHistory()) {
            if (msg.getTargetId().empty()) {
                total_pub++;
            }
            else {
                total_sub++;
            }
        }

        ImGui::Text("Statistics:");
        ImGui::Text("  Publish Messages: %zu", total_pub);
        ImGui::Text("  Subscribe Deliveries: %zu", total_sub);

        // Calculate and show messaging rate (messages per second)
        if (!broker->getMessageHistory().empty()) {
            auto oldest = broker->getMessageHistory().front().getTimestamp();
            auto newest = broker->getMessageHistory().back().getTimestamp();

            auto duration = std::chrono::duration_cast<std::chrono::seconds>(newest - oldest);
            if (duration.count() > 0) {
                float rate = broker->getMessageHistory().size() / static_cast<float>(duration.count());
                ImGui::Text("  Messaging Rate: %.1f msg/sec", rate);
            }
        }
    }


} // namespace visualization