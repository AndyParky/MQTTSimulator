#pragma once

#include "Message.h"
#include "Constants.h"
#include <string>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <chrono>
#include <memory>
#include <functional>

namespace mqtt {

    // Forward declaration
    class Broker;

    /**
     * @brief MQTT Client Device class
     */
    class Device : public std::enable_shared_from_this<Device> {
    public:
        /**
         * @brief Construct a new Device object
         */
        Device(const std::string& id,
            std::shared_ptr<Broker> broker,
            std::chrono::milliseconds interval = std::chrono::milliseconds(mqtt::constants::GATEWAY_INTERVAL_MS));

        /**
         * @brief Destroy the Device object
         */
        ~Device();

        // Delete copy and move constructors/operators
        Device(const Device&) = delete;
        Device& operator=(const Device&) = delete;
        Device(Device&&) = delete;
        Device& operator=(Device&&) = delete;

        // MQTT operations
        void subscribe(const std::string& topic);
        void unsubscribe(const std::string& topic);
        void publish(const std::string& topic,
            const std::string& payload,
            QoS qos = QoS::AT_MOST_ONCE,
            bool retained = false);
        void receiveMessage(const Message& message);

        // Message handling
        void addMessageHandler(std::function<void(const Message&)> handler);

        // Accessors
        const std::string& getId() const;
        const std::vector<Message>& getMessageHistory() const;
        const std::vector<std::string>& getSubscribedTopics() const;

        // Configuration
        void setTelemetryInterval(std::chrono::milliseconds interval);

    private:
        void generateTelemetry();
        std::string generateRandomTelemetry();

    private:
        std::string device_id;
        std::weak_ptr<Broker> broker;
        std::vector<std::string> subscribed_topics;
        std::queue<Message> received_messages;
        std::mutex mutex;
        std::vector<std::function<void(const Message&)>> message_handlers;

        // For telemetry simulation
        std::thread telemetry_thread;
        std::atomic<bool> running;
        std::chrono::milliseconds telemetry_interval;

        // For visualization
        std::vector<Message> message_history;
        const size_t MAX_HISTORY_SIZE = mqtt::constants::DEVICE_MESSAGE_HISTORY_SIZE;
    };

} // namespace mqtt