#include <gtest/gtest.h>
#include <cstring>
#include "../src/serialize.h"
#include "../src/deserialize.h"
#include "../src/messages.h"

class SerializeDeserializeTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Initialize common test data here
    }

    void TearDown() override {
        // Cleanup if needed
    }

    // Helper to compare two strings (including NULL-termination)
    bool compareStrings(const char* str1, const char* str2) {
        if (str1 == nullptr && str2 == nullptr) return true;
        if (str1 == nullptr || str2 == nullptr) return false;
        return strcmp(str1, str2) == 0;
    }
};

// CONFIRM Message Tests
TEST_F(SerializeDeserializeTest, ConfirmMsgSerializationDeserialization) {
    // Create original message
    struct Confirm_MSG original_msg;
    create_confirm_msg(&original_msg, 12345);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_confirm_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check (1 byte type + 2 bytes message ID)
    EXPECT_EQ(serialized_size, 3);
    
    // Verify first byte (type)
    EXPECT_EQ(serialized_data[0], MSG_TYPE_VAL[CONFIRM]);
    
    // Verify message ID
    uint16_t serialized_msg_id = (serialized_data[1] << 8) | serialized_data[2];
    EXPECT_EQ(serialized_msg_id, 12345);
    
    // Deserialize
    struct Confirm_MSG* deserialized_msg = deserialize_confirm_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[CONFIRM]);
    EXPECT_EQ(deserialized_msg->ref_message_id, 12345);
    
    // Cleanup
    free(serialized_data);
    free_confirm_msg(deserialized_msg);
}

// REPLY Message Tests
TEST_F(SerializeDeserializeTest, ReplyMsgSerializationDeserialization) {
    // Create original message
    struct Reply_MSG original_msg;
    const char* test_content = "Authentication success.";
    create_reply_msg(&original_msg, 1, 1, 0, (char*)test_content);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_reply_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check (1 byte type + 2 bytes messageID + 1 byte result + 2 bytes ref_messageID + content length + null terminator)
    size_t expected_size = 1 + 2 + 1 + 2 + strlen(test_content) + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct Reply_MSG* deserialized_msg = deserialize_reply_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[REPLY]);
    EXPECT_EQ(deserialized_msg->message_id, 1);
    EXPECT_EQ(deserialized_msg->result, 1);
    EXPECT_EQ(deserialized_msg->ref_message_id, 0);
    EXPECT_TRUE(compareStrings(deserialized_msg->message_contents, test_content));
    
    // Cleanup
    free(serialized_data);
    free_reply_msg(deserialized_msg);
}

// AUTH Message Tests
TEST_F(SerializeDeserializeTest, AuthMsgSerializationDeserialization) {
    // Create original message
    struct Auth_MSG original_msg;
    const char* username = "testuser";
    const char* display_name = "Test User";
    const char* secret = "password123";
    create_auth_msg(&original_msg, (char*)username, (char*)display_name, (char*)secret);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_auth_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check (1 byte type + 2 bytes messageID + strings with null terminators)
    size_t expected_size = 1 + 2 + strlen(username) + 1 + strlen(display_name) + 1 + strlen(secret) + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct Auth_MSG* deserialized_msg = deserialize_auth_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[AUTH]);
    EXPECT_TRUE(compareStrings(deserialized_msg->username, username));
    EXPECT_TRUE(compareStrings(deserialized_msg->display_name, display_name));
    EXPECT_TRUE(compareStrings(deserialized_msg->secret, secret));
    
    // Cleanup
    free(serialized_data);
    free_auth_msg(deserialized_msg);
}

// JOIN Message Tests
TEST_F(SerializeDeserializeTest, JoinMsgSerializationDeserialization) {
    // Create original message
    struct Join_MSG original_msg;
    const char* channel_id = "general";
    const char* display_name = "Test User";
    create_join_msg(&original_msg, 42, (char*)channel_id, (char*)display_name);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_join_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check
    size_t expected_size = 1 + 2 + strlen(channel_id) + 1 + strlen(display_name) + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct Join_MSG* deserialized_msg = deserialize_join_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[JOIN]);
    EXPECT_EQ(deserialized_msg->message_id, 42);
    EXPECT_TRUE(compareStrings(deserialized_msg->channel_id, channel_id));
    EXPECT_TRUE(compareStrings(deserialized_msg->display_name, display_name));
    
    // Cleanup
    free(serialized_data);
    free_join_msg(deserialized_msg);
}

// Regular Message Tests
TEST_F(SerializeDeserializeTest, RegularMsgSerializationDeserialization) {
    // Create original message
    struct MSG original_msg;
    const char* display_name = "Alice";
    const char* message_content = "Hello, world!";
    create_msg(&original_msg, 7, (char*)display_name, (char*)message_content);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check
    size_t expected_size = 1 + 2 + strlen(display_name) + 1 + strlen(message_content) + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct MSG* deserialized_msg = deserialize_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[MSG]);
    EXPECT_EQ(deserialized_msg->message_id, 7);
    EXPECT_TRUE(compareStrings(deserialized_msg->display_name, display_name));
    EXPECT_TRUE(compareStrings(deserialized_msg->message_contents, message_content));
    
    // Cleanup
    free(serialized_data);
    free_msg(deserialized_msg);
}

// ERROR Message Tests
TEST_F(SerializeDeserializeTest, ErrorMsgSerializationDeserialization) {
    // Create original message
    struct Err_MSG original_msg;
    const char* display_name = "Server";
    const char* error_message = "Invalid channel name";
    create_err_msg(&original_msg, 15, (char*)display_name, (char*)error_message);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_err_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check
    size_t expected_size = 1 + 2 + strlen(display_name) + 1 + strlen(error_message) + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct Err_MSG* deserialized_msg = deserialize_err_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[ERR]);
    EXPECT_EQ(deserialized_msg->message_id, 15);
    EXPECT_TRUE(compareStrings(deserialized_msg->display_name, display_name));
    EXPECT_TRUE(compareStrings(deserialized_msg->message_contents, error_message));
    
    // Cleanup
    free(serialized_data);
    free_err_msg(deserialized_msg);
}

// BYE Message Tests
TEST_F(SerializeDeserializeTest, ByeMsgSerializationDeserialization) {
    // Create original message
    struct Bye_MSG original_msg;
    const char* display_name = "User123";
    create_bye_msg(&original_msg, 99, (char*)display_name);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_bye_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check
    size_t expected_size = 1 + 2 + strlen(display_name) + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct Bye_MSG* deserialized_msg = deserialize_bye_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[BYE]);
    EXPECT_EQ(deserialized_msg->message_id, 99);
    EXPECT_TRUE(compareStrings(deserialized_msg->display_name, display_name));
    
    // Cleanup
    free(serialized_data);
    free_bye_msg(deserialized_msg);
}

// PING Message Tests
TEST_F(SerializeDeserializeTest, PingMsgSerializationDeserialization) {
    // Create original message
    struct Ping_MSG original_msg;
    create_ping_msg(&original_msg, 123);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_ping_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check (1 byte type + 2 bytes message ID)
    EXPECT_EQ(serialized_size, 3);
    
    // Verify first byte (type)
    EXPECT_EQ(serialized_data[0], MSG_TYPE_VAL[PING]);
    
    // Deserialize
    struct Ping_MSG* deserialized_msg = deserialize_ping_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the deserialized message
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[PING]);
    EXPECT_EQ(deserialized_msg->message_id, 123);
    
    // Cleanup
    free(serialized_data);
    free_ping_msg(deserialized_msg);
}

// Edge cases tests
TEST_F(SerializeDeserializeTest, EmptyStringsHandling) {
    // Create message with empty strings
    struct MSG original_msg;
    const char* empty_name = "";
    const char* empty_content = "";
    create_msg(&original_msg, 8, (char*)empty_name, (char*)empty_content);
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check (1 byte type + 2 bytes messageID + 2 null terminators)
    EXPECT_EQ(serialized_size, 1 + 2 + 1 + 1);
    
    // Deserialize
    struct MSG* deserialized_msg = deserialize_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify empty strings are preserved
    EXPECT_TRUE(compareStrings(deserialized_msg->display_name, empty_name));
    EXPECT_TRUE(compareStrings(deserialized_msg->message_contents, empty_content));
    
    // Cleanup
    free(serialized_data);
    free_msg(deserialized_msg);
}

TEST_F(SerializeDeserializeTest, LongMessageContent) {
    // Create message with long content (simulating max length)
    struct MSG original_msg;
    const char* display_name = "TestUser";
    
    // Create a long string (not the full 60000 for test efficiency)
    std::string long_content(1000, 'A'); // 1000 'A' characters
    
    create_msg(&original_msg, 9, (char*)display_name, (char*)long_content.c_str());
    
    // Serialize
    size_t serialized_size;
    uint8_t* serialized_data = serialize_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Expected size check
    size_t expected_size = 1 + 2 + strlen(display_name) + 1 + long_content.length() + 1;
    EXPECT_EQ(serialized_size, expected_size);
    
    // Deserialize
    struct MSG* deserialized_msg = deserialize_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify content length
    EXPECT_EQ(strlen(deserialized_msg->message_contents), long_content.length());
    
    // Cleanup
    free(serialized_data);
    free_msg(deserialized_msg);
}

TEST_F(SerializeDeserializeTest, InvalidBufferSize) {
    // Create and serialize a message
    struct MSG original_msg;
    const char* display_name = "User";
    const char* message_content = "Test message";
    create_msg(&original_msg, 10, (char*)display_name, (char*)message_content);
    
    size_t serialized_size;
    uint8_t* serialized_data = serialize_msg(&original_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Try deserializing with insufficient buffer size
    struct MSG* deserialized_msg = deserialize_msg(serialized_data, 3); // Just the header
    
    // Depending on your implementation, this should either return NULL or a partial message
    // Adjust this expectation based on your implementation's behavior
    EXPECT_EQ(deserialized_msg, nullptr);
    
    // Cleanup
    free(serialized_data);
    if (deserialized_msg != nullptr) {
        free_msg(deserialized_msg);
    }
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
