#include "../include/Broker.h"
#include "../include/Device.h"
#include <algorithm>
#include <regex>

namespace mqtt {

    Broker::Broker(const std::string& id)
        : broker_id(id), running(true) {
        processing_thread = std::thread(&Broker::processMessages, this);
    }

    Broker::~Broker() {
        running = false;
        if (processing_thread.joinable()) {
            processing_thread.join();
        }
    }

    void Broker::subscribe(const std::string& topic, std::shared_ptr<Device> device) {
        std::lock_guard<std::mutex> lock(mutex);
        topic_subscriptions[topic].push_back(device);

        // Send any retained messages that match the subscription
        for (const auto& retained : retained_messages) {
            if (topicMatches(topic, retained.first)) {
                device->receiveMessage(retained.second);
            }
        }
    }

    void Broker::unsubscribe(const std::string& topic, std::shared_ptr<Device> device) {
        std::lock_guard<std::mutex> lock(mutex);
        auto& subscribers = topic_subscriptions[topic];
        subscribers.erase(
            std::remove_if(subscribers.begin(), subscribers.end(),
                [&device](const std::weak_ptr<Device>& wp) {
                    auto sp = wp.lock();
                    return !sp || sp.get() == device.get();
                }),
            subscribers.end());
    }

    void Broker::publish(const Message& message) {
        std::lock_guard<std::mutex> lock(mutex);
        message_queue.push(message);

        // Store retained messages
        if (message.isRetained()) {
            retained_messages[message.getTopic()] = message;
        }

        // Add to history for visualization
        message_history.push_back(message);
        if (message_history.size() > MAX_HISTORY_SIZE) {
            message_history.erase(message_history.begin());
        }
    }

    const std::vector<Message>& Broker::getMessageHistory() const {
        return message_history;
    }

    const std::string& Broker::getId() const {
        return broker_id;
    }

    void Broker::processMessages() {
        while (running) {
            Message message;
            {
                std::lock_guard<std::mutex> lock(mutex);
                if (!message_queue.empty()) {
                    message = message_queue.front();
                    message_queue.pop();
                }
            }

            if (!message.getTopic().empty()) {
                distributeMessage(message);
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }

    void Broker::distributeMessage(const Message& message) {
        std::lock_guard<std::mutex> lock(mutex);

        for (const auto& subscription : topic_subscriptions) {
            if (topicMatches(subscription.first, message.getTopic())) {
                for (auto& weak_device : subscription.second) {
                    if (auto device = weak_device.lock()) {
                        Message outgoing_message = message;
                        outgoing_message.setTargetId(device->getId());
                        device->receiveMessage(outgoing_message);
                    }
                }
            }
        }
    }

    bool Broker::topicMatches(const std::string& subscription, const std::string& topic) {
        // Exact match
        if (subscription == topic) {
            return true;
        }

        // Handle single-level wildcard +
        if (subscription.find('+') != std::string::npos) {
            std::string sub_regex = subscription;
            size_t pos = 0;
            while ((pos = sub_regex.find('+', pos)) != std::string::npos) {
                sub_regex.replace(pos, 1, "[^/]+");
                pos += 5;
            }

            // Very basic regex matching - not efficient but works for demo
            return std::regex_match(topic, std::regex(sub_regex));
        }

        // Handle multi-level wildcard #
        if (subscription.back() == '#') {
            std::string prefix = subscription.substr(0, subscription.length() - 1);
            return topic.substr(0, prefix.length()) == prefix;
        }

        return false;
    }

} // namespace mqtt