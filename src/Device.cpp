#include "Device.h"
#include "Broker.h"
#include <algorithm>
#include <random>
#include <sstream>
#include <iomanip>

namespace mqtt {

    Device::Device(const std::string& id,
        std::shared_ptr<Broker> broker,
        std::chrono::milliseconds interval)
        : device_id(id),
        broker(broker),
        running(true),
        telemetry_interval(interval) {
        telemetry_thread = std::thread(&Device::generateTelemetry, this);
    }

    Device::~Device() {
        running = false;
        if (telemetry_thread.joinable()) {
            telemetry_thread.join();
        }
    }

    void Device::subscribe(const std::string& topic) {
        if (auto b = broker.lock()) {
            b->subscribe(topic, shared_from_this());
            subscribed_topics.push_back(topic);
        }
    }

    void Device::unsubscribe(const std::string& topic) {
        if (auto b = broker.lock()) {
            b->unsubscribe(topic, shared_from_this());
            auto it = std::find(subscribed_topics.begin(), subscribed_topics.end(), topic);
            if (it != subscribed_topics.end()) {
                subscribed_topics.erase(it);
            }
        }
    }

    void Device::publish(const std::string& topic, const std::string& payload,
        QoS qos, bool retained) {
        if (auto b = broker.lock()) {
            Message message(topic, payload, qos, retained);
            message.setSenderId(device_id);

            // Add to history for visualization
            {
                std::lock_guard<std::mutex> lock(mutex);
                message_history.push_back(message);
                if (message_history.size() > MAX_HISTORY_SIZE) {
                    message_history.erase(message_history.begin());
                }
            }

            b->publish(message);
        }
    }

    void Device::receiveMessage(const Message& message) {
        std::lock_guard<std::mutex> lock(mutex);
        received_messages.push(message);

        // Add to history for visualization
        message_history.push_back(message);
        if (message_history.size() > MAX_HISTORY_SIZE) {
            message_history.erase(message_history.begin());
        }

        // Process message with registered handlers
        for (const auto& handler : message_handlers) {
            handler(message);
        }
    }

    void Device::addMessageHandler(std::function<void(const Message&)> handler) {
        message_handlers.push_back(handler);
    }

    const std::string& Device::getId() const {
        return device_id;
    }

    const std::vector<Message>& Device::getMessageHistory() const {
        return message_history;
    }

    const std::vector<std::string>& Device::getSubscribedTopics() const {
        return subscribed_topics;
    }

    void Device::setTelemetryInterval(std::chrono::milliseconds interval) {
        telemetry_interval = interval;
    }

    void Device::generateTelemetry() {
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> interval_var(mqtt::constants::TELEMETRY_RANDOM_MIN_MS, mqtt::constants::TELEMETRY_RANDOM_MAX_MS);

        while (running) {
            // Generate and publish telemetry
            std::string telemetry = generateRandomTelemetry();
            publish("telemetry/" + device_id, telemetry, QoS::AT_LEAST_ONCE);

            // Random variation in telemetry timing for realism
            std::this_thread::sleep_for(telemetry_interval +
                std::chrono::milliseconds(interval_var(gen)));
        }
    }

    std::string Device::generateRandomTelemetry() {
        std::random_device rd;
        std::mt19937 gen(rd());

        // Generate sensor values
        std::uniform_real_distribution<> temp(mqtt::constants::TEMPERATURE_MIN, mqtt::constants::TEMPERATURE_MAX);
        std::uniform_real_distribution<> humidity(mqtt::constants::HUMIDITY_MIN, mqtt::constants::HUMIDITY_MAX);
        std::uniform_real_distribution<> pressure(mqtt::constants::PRESSURE_MIN, mqtt::constants::PRESSURE_MAX);
        std::uniform_real_distribution<> battery(mqtt::constants::BATTERY_MIN, mqtt::constants::BATTERY_MAX);

        // Create JSON-like output
        std::stringstream ss;
        ss << "{"
            << "\"temperature\":" << std::fixed << std::setprecision(1) << temp(gen) << ","
            << "\"humidity\":" << std::fixed << std::setprecision(1) << humidity(gen) << ","
            << "\"pressure\":" << std::fixed << std::setprecision(1) << pressure(gen) << ","
            << "\"battery\":" << std::fixed << std::setprecision(2) << battery(gen) << ","
            << "\"timestamp\":\"" << std::chrono::system_clock::now().time_since_epoch().count() << "\""
            << "}";

        return ss.str();
    }

} // namespace mqtt