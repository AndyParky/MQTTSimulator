#pragma once

namespace mqtt {

    /**
     * @brief MQTT Quality of Service levels
     */
    enum class QoS {
        AT_MOST_ONCE = 0,    // Fire and forget (0)
        AT_LEAST_ONCE = 1,   // Acknowledged delivery (1)
        EXACTLY_ONCE = 2     // Assured delivery (2)
    };

} // namespace mqtt