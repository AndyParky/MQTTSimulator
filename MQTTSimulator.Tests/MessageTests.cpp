#include "pch.h"
#include "Message.h"

using namespace mqtt;

class MessageTest : public ::testing::Test {
protected:
    // Setup method runs before each test
    void SetUp() override {
        // Common setup code
    }

    // Teardown method runs after each test
    void TearDown() override {
        // Common cleanup code
    }
};

// Test constructor
TEST_F(MessageTest, Constructor_ValidInput_SetsProperties) {
    // Arrange
    std::string testTopic = "test/topic";
    std::string testPayload = "test payload";

    // Act
    Message message(testTopic, testPayload);

    // Assert
    EXPECT_EQ(testTopic, message.getTopic());
    EXPECT_EQ(testPayload, message.getPayload());
}

// Test setters
TEST_F(MessageTest, Setters_ValidInput_ChangesProperties) {
    // Arrange
    Message message;
    std::string newTopic = "new/topic";

    // Act
    message.setTopic(newTopic);

    // Assert
    EXPECT_EQ(newTopic, message.getTopic());
}