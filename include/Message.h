#pragma once

#include "QoS.h"
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <cstdint>

namespace mqtt {

    /**
     * @brief MQTT Message class implementing MQTT 5.0 message format
     */
    class Message {
    public:
        /**
         * @brief Construct a new Message object
         */
        Message(const std::string& topic = "",
            const std::string& payload = "",
            QoS qos = QoS::AT_MOST_ONCE,
            bool retained = false);

        // Getters and setters
        const std::string& getTopic() const;
        void setTopic(const std::string& topic);

        const std::string& getPayload() const;
        void setPayload(const std::string& payload);

        QoS getQoS() const;
        void setQoS(QoS qos);

        bool isRetained() const;
        void setRetained(bool retained);

        const std::string& getSenderId() const;
        void setSenderId(const std::string& sender_id);

        const std::string& getTargetId() const;
        void setTargetId(const std::string& target_id);

        std::chrono::system_clock::time_point getTimestamp() const;

        // MQTT 5.0 specific properties
        void addUserProperty(const std::string& key, const std::string& value);
        const std::map<std::string, std::string>& getUserProperties() const;

        void setMessageExpiryInterval(uint32_t interval);
        uint32_t getMessageExpiryInterval() const;

        void setTopicAlias(uint16_t alias);
        uint16_t getTopicAlias() const;

        void setContentType(const std::string& content_type);
        const std::string& getContentType() const;

        void setResponseTopic(const std::string& response_topic);
        const std::string& getResponseTopic() const;

        void setCorrelationData(const std::vector<uint8_t>& correlation_data);
        const std::vector<uint8_t>& getCorrelationData() const;

    private:
        std::string topic;
        std::string payload;
        QoS qos;
        bool retained;
        std::string sender_id;
        std::string target_id; // For visualization purposes
        std::chrono::system_clock::time_point timestamp;

        // MQTT 5.0 specific properties
        std::map<std::string, std::string> user_properties;
        uint32_t message_expiry_interval;
        uint16_t topic_alias;
        std::string content_type;
        std::string response_topic;
        std::vector<uint8_t> correlation_data;
    };

} // namespace mqtt