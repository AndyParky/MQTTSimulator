#include "pch.h"
#include "Message.h"
#include "Broker.h"
#include "Device.h"

using namespace mqtt;

// Message Tests
TEST(MessageTests, ConstructorSetsProperties) {
	// Arrange
	std::string testTopic = "test/topic";
	std::string testPayload = "test payload";
	QoS testQoS = QoS::AT_LEAST_ONCE;
	bool testRetained = true;

	// Act
	Message message(testTopic, testPayload, testQoS, testRetained);

	// Assert
	EXPECT_EQ(testTopic, message.getTopic());
	EXPECT_EQ(testPayload, message.getPayload());
	EXPECT_EQ(testQoS, message.getQoS());
	EXPECT_TRUE(message.isRetained());
}

// Broker Tests
TEST(BrokerTests, GetIdReturnsCorrectId) {
	// Arrange
	auto broker = std::make_shared<Broker>("test_broker");

	// Act & Assert
	EXPECT_EQ("test_broker", broker->getId());
}

// Device Tests
TEST(DeviceTests, GetIdReturnsCorrectId) {
	// Arrange
	auto broker = std::make_shared<Broker>("test_broker");
	auto device = std::make_shared<Device>("test_device", broker);

	// Act & Assert
	EXPECT_EQ("test_device", device->getId());
}