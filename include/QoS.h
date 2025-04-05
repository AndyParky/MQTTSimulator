#pragma once

namespace mqtt {

    /**
     * @brief MQTT Quality of Service levels
     */
    enum class QoS {
        AT_MOST_ONCE = 0,    // No acknowledgement (0)
        AT_LEAST_ONCE = 1,   // Ack. delivery (1)
        EXACTLY_ONCE = 2     // Guaranteed delivery (2)
    };

} // namespace mqtt