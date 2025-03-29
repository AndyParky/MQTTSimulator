#include "../include/Message.h"

namespace mqtt {

    Message::Message(const std::string& topic,
        const std::string& payload,
        QoS qos,
        bool retained)
        : topic(topic),
        payload(payload),
        qos(qos),
        retained(retained),
        timestamp(std::chrono::system_clock::now()),
        message_expiry_interval(0),
        topic_alias(0) {
    }

    const std::string& Message::getTopic() const {
        return topic;
    }

    void Message::setTopic(const std::string& topic) {
        this->topic = topic;
    }

    const std::string& Message::getPayload() const {
        return payload;
    }

    void Message::setPayload(const std::string& payload) {
        this->payload = payload;
    }

    QoS Message::getQoS() const {
        return qos;
    }

    void Message::setQoS(QoS qos) {
        this->qos = qos;
    }

    bool Message::isRetained() const {
        return retained;
    }

    void Message::setRetained(bool retained) {
        this->retained = retained;
    }

    const std::string& Message::getSenderId() const {
        return sender_id;
    }

    void Message::setSenderId(const std::string& sender_id) {
        this->sender_id = sender_id;
    }

    const std::string& Message::getTargetId() const {
        return target_id;
    }

    void Message::setTargetId(const std::string& target_id) {
        this->target_id = target_id;
    }

    std::chrono::system_clock::time_point Message::getTimestamp() const {
        return timestamp;
    }

    void Message::addUserProperty(const std::string& key, const std::string& value) {
        user_properties[key] = value;
    }

    const std::map<std::string, std::string>& Message::getUserProperties() const {
        return user_properties;
    }

    void Message::setMessageExpiryInterval(uint32_t interval) {
        message_expiry_interval = interval;
    }

    uint32_t Message::getMessageExpiryInterval() const {
        return message_expiry_interval;
    }

    void Message::setTopicAlias(uint16_t alias) {
        topic_alias = alias;
    }

    uint16_t Message::getTopicAlias() const {
        return topic_alias;
    }

    void Message::setContentType(const std::string& content_type) {
        this->content_type = content_type;
    }

    const std::string& Message::getContentType() const {
        return content_type;
    }

    void Message::setResponseTopic(const std::string& response_topic) {
        this->response_topic = response_topic;
    }

    const std::string& Message::getResponseTopic() const {
        return response_topic;
    }

    void Message::setCorrelationData(const std::vector<uint8_t>& correlation_data) {
        this->correlation_data = correlation_data;
    }

    const std::vector<uint8_t>& Message::getCorrelationData() const {
        return correlation_data;
    }

} // namespace mqtt