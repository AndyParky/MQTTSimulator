#pragma once

#include "Message.h"
#include "Constants.h"
#include <string>
#include <map>
#include <vector>
#include <queue>
#include <mutex>
#include <thread>
#include <atomic>
#include <memory>

namespace mqtt {

    // Forward declaration
    class Device;

    /**
     * @brief MQTT Broker class
     */
    class Broker {
    public:
        /**
         * @brief Construct a new Broker object
         */
        explicit Broker(const std::string& id);

        /**
         * @brief Destroy the Broker object
         */
        ~Broker();

        // Delete copy and move constructors/operators
        Broker(const Broker&) = delete;
        Broker& operator=(const Broker&) = delete;
        Broker(Broker&&) = delete;
        Broker& operator=(Broker&&) = delete;

        // Subscription management
        void subscribe(const std::string& topic, std::shared_ptr<Device> device);
        void unsubscribe(const std::string& topic, std::shared_ptr<Device> device);

        // Message handling
        void publish(const Message& message);

        // Accessors
        const std::vector<Message>& getMessageHistory() const;
        const std::string& getId() const;

    private:
        void processMessages();
        void distributeMessage(const Message& message);
        bool topicMatches(const std::string& subscription, const std::string& topic);

    private:
        std::string broker_id;
        std::map<std::string, std::vector<std::weak_ptr<Device>>> topic_subscriptions;
        std::map<std::string, Message> retained_messages;
        std::queue<Message> message_queue;
        std::mutex mutex;
        std::thread processing_thread;
        std::atomic<bool> running;

        // For visualization
        std::vector<Message> message_history;
        const size_t MAX_HISTORY_SIZE = mqtt::constants::BROKER_MESSAGE_HISTORY_SIZE;
    };

} // namespace mqtt