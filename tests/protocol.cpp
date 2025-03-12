#include <gtest/gtest.h>
#include <cstring>
#include "../src/serialize.h"
#include "../src/deserialize.h"
#include "../src/messages.h"

// Test fixture for protocol validation
class ProtocolValidationTest : public ::testing::Test {
protected:
    void SetUp() override {
        // Common setup for protocol validation tests
    }
    
    void TearDown() override {
        // Cleanup
    }
    
    // Helper to create a complete message sequence for testing
    uint8_t* createAuthSequence(size_t* size, uint16_t client_msg_id) {
        // Create an AUTH message
        struct Auth_MSG auth_msg;
        create_auth_msg(&auth_msg, (char*)"testuser", (char*)"Display Name", (char*)"password123");
        auth_msg.message_id = client_msg_id;
        
        return serialize_auth_msg(&auth_msg, size);
    }
    
    uint8_t* createConfirmSequence(size_t* size, uint16_t ref_msg_id) {
        // Create a CONFIRM message
        struct Confirm_MSG confirm_msg;
        create_confirm_msg(&confirm_msg, ref_msg_id);
        
        return serialize_confirm_msg(&confirm_msg, size);
    }
    
    uint8_t* createReplySequence(size_t* size, uint16_t server_msg_id, uint8_t result, uint16_t ref_msg_id) {
        // Create a REPLY message
        struct Reply_MSG reply_msg;
        create_reply_msg(&reply_msg, server_msg_id, result, ref_msg_id, 
                        result == 1 ? (char*)"Authentication success." : (char*)"Authentication failed.");
        
        return serialize_reply_msg(&reply_msg, size);
    }
};

// Test protocol message field constraints
TEST_F(ProtocolValidationTest, MessageFieldConstraints) {
    // Test username length constraint (max 20 chars)
    struct Auth_MSG auth_msg;
    const char* valid_username = "validusername123456"; // 18 chars
    const char* invalid_username = "thisusernameiswaywaytoolong"; // 26 chars
    const char* display_name = "Display Name";
    const char* secret = "password";
    
    // Valid username
    create_auth_msg(&auth_msg, (char*)valid_username, (char*)display_name, (char*)secret);
    
    size_t serialized_size;
    uint8_t* serialized_data = serialize_auth_msg(&auth_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Your serialize function might not validate these constraints,
    // so we test the deserialized data for field correctness
    struct Auth_MSG* deserialized_msg = deserialize_auth_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    EXPECT_TRUE(strlen(deserialized_msg->username) <= 20);
    
    free(serialized_data);
    free_auth_msg(deserialized_msg);
    
    // For invalid username, behavior depends on your implementation:
    // Option 1: Your create_auth_msg truncates too long usernames
    // Option 2: Your create_auth_msg rejects invalid usernames
    // Option 3: Your serialize function handles the validation
    
    // This test assumes Option 1 or 3. If using Option 2, adjust accordingly.
    create_auth_msg(&auth_msg, (char*)invalid_username, (char*)display_name, (char*)secret);
    
    serialized_data = serialize_auth_msg(&auth_msg, &serialized_size);
    if (serialized_data != nullptr) {
        // If serialization succeeded, ensure username is truncated or handled
        deserialized_msg = deserialize_auth_msg(serialized_data, serialized_size);
        ASSERT_NE(deserialized_msg, nullptr);
        EXPECT_TRUE(strlen(deserialized_msg->username) <= 20);
        
        free(serialized_data);
        free_auth_msg(deserialized_msg);
    } else {
        // If serialization rejected invalid input, this is also valid behavior
        SUCCEED();
    }
}

// Test protocol character set validation
TEST_F(ProtocolValidationTest, CharacterSetValidation) {
    // Test username character set (only [a-zA-Z0-9_-] allowed)
    struct Auth_MSG auth_msg;
    const char* valid_username = "Valid-User_123";
    const char* invalid_username = "Invalid@User!";
    const char* display_name = "Display Name"; // Display name allows all printable chars
    const char* secret = "password123";
    
    // Valid username
    create_auth_msg(&auth_msg, (char*)valid_username, (char*)display_name, (char*)secret);
    
    size_t serialized_size;
    uint8_t* serialized_data = serialize_auth_msg(&auth_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    struct Auth_MSG* deserialized_msg = deserialize_auth_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Verify the username was preserved correctly
    EXPECT_STREQ(deserialized_msg->username, valid_username);
    
    free(serialized_data);
    free_auth_msg(deserialized_msg);
    
    // For invalid username, behavior depends on your implementation
    // Similar to the previous test, handle according to your implementation approach
}

// Test protocol message sequence
TEST_F(ProtocolValidationTest, MessageSequence) {
    // Test a simple authentication sequence
    // CLIENT: AUTH -> SERVER: CONFIRM -> SERVER: REPLY -> CLIENT: CONFIRM
    
    // 1. Client sends AUTH
    size_t auth_size;
    uint16_t client_msg_id = 0;
    uint8_t* auth_data = createAuthSequence(&auth_size, client_msg_id);
    ASSERT_NE(auth_data, nullptr);
    
    // Verify auth message format
    struct Auth_MSG* auth_msg = deserialize_auth_msg(auth_data, auth_size);
    ASSERT_NE(auth_msg, nullptr);
    EXPECT_EQ(auth_msg->type, MSG_TYPE_VAL[AUTH]);
    EXPECT_EQ(auth_msg->message_id, client_msg_id);
    
    // 2. Server sends CONFIRM
    size_t confirm_size;
    uint8_t* confirm_data = createConfirmSequence(&confirm_size, client_msg_id);
    ASSERT_NE(confirm_data, nullptr);
    
    // Verify confirm message format
    struct Confirm_MSG* confirm_msg = deserialize_confirm_msg(confirm_data, confirm_size);
    ASSERT_NE(confirm_msg, nullptr);
    EXPECT_EQ(confirm_msg->type, MSG_TYPE_VAL[CONFIRM]);
    EXPECT_EQ(confirm_msg->ref_message_id, client_msg_id);
    
    // 3. Server sends REPLY
    size_t reply_size;
    uint16_t server_msg_id = 0;
    uint8_t result = 1; // Success
    uint8_t* reply_data = createReplySequence(&reply_size, server_msg_id, result, client_msg_id);
    ASSERT_NE(reply_data, nullptr);
    
    // Verify reply message format
    struct Reply_MSG* reply_msg = deserialize_reply_msg(reply_data, reply_size);
    ASSERT_NE(reply_msg, nullptr);
    EXPECT_EQ(reply_msg->type, MSG_TYPE_VAL[REPLY]);
    EXPECT_EQ(reply_msg->message_id, server_msg_id);
    EXPECT_EQ(reply_msg->result, result);
    EXPECT_EQ(reply_msg->ref_message_id, client_msg_id);
    
    // 4. Client sends CONFIRM for the reply
    size_t client_confirm_size;
    uint8_t* client_confirm_data = createConfirmSequence(&client_confirm_size, server_msg_id);
    ASSERT_NE(client_confirm_data, nullptr);
    
    // Verify confirm message format
    struct Confirm_MSG* client_confirm_msg = deserialize_confirm_msg(client_confirm_data, client_confirm_size);
    ASSERT_NE(client_confirm_msg, nullptr);
    EXPECT_EQ(client_confirm_msg->type, MSG_TYPE_VAL[CONFIRM]);
    EXPECT_EQ(client_confirm_msg->ref_message_id, server_msg_id);
    
    // Clean up
    free(auth_data);
    free(confirm_data);
    free(reply_data);
    free(client_confirm_data);
    free_auth_msg(auth_msg);
    free_confirm_msg(confirm_msg);
    free_reply_msg(reply_msg);
    free_confirm_msg(client_confirm_msg);
}

// Test protocol error handling
TEST_F(ProtocolValidationTest, ErrorHandling) {
    // Test error message creation and validation
    struct Err_MSG err_msg;
    const char* display_name = "Server";
    const char* error_content = "Protocol error: Invalid message format";
    uint16_t message_id = 5;
    
    create_err_msg(&err_msg, message_id, (char*)display_name, (char*)error_content);
    
    size_t serialized_size;
    uint8_t* serialized_data = serialize_err_msg(&err_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Verify error message format
    struct Err_MSG* deserialized_msg = deserialize_err_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[ERR]);
    EXPECT_EQ(deserialized_msg->message_id, message_id);
    EXPECT_STREQ(deserialized_msg->display_name, display_name);
    EXPECT_STREQ(deserialized_msg->message_contents, error_content);
    
    free(serialized_data);
    free_err_msg(deserialized_msg);
}

// Test protocol termination
TEST_F(ProtocolValidationTest, ProtocolTermination) {
    // Test BYE message creation and validation
    struct Bye_MSG bye_msg;
    const char* display_name = "Test User";
    uint16_t message_id = 100;
    
    create_bye_msg(&bye_msg, message_id, (char*)display_name);
    
    size_t serialized_size;
    uint8_t* serialized_data = serialize_bye_msg(&bye_msg, &serialized_size);
    ASSERT_NE(serialized_data, nullptr);
    
    // Verify BYE message format
    struct Bye_MSG* deserialized_msg = deserialize_bye_msg(serialized_data, serialized_size);
    ASSERT_NE(deserialized_msg, nullptr);
    EXPECT_EQ(deserialized_msg->type, MSG_TYPE_VAL[BYE]);
    EXPECT_EQ(deserialized_msg->message_id, message_id);
    EXPECT_STREQ(deserialized_msg->display_name, display_name);
    
    free(serialized_data);
    free_bye_msg(deserialized_msg);
}

// Test reserialization of deserialized message
TEST_F(ProtocolValidationTest, RoundTripConsistency) {
    // Create a message, serialize, deserialize, then serialize again
    // The two serialized forms should be identical
    
    struct MSG original_msg;
    const char* display_name = "Alice";
    const char* message_content = "This is a test message for consistency";
    create_msg(&original_msg, 42, (char*)display_name, (char*)message_content);
    
    // First serialization
    size_t original_size;
    uint8_t* original_data = serialize_msg(&original_msg, &original_size);
    ASSERT_NE(original_data, nullptr);
    
    // Save a copy
    uint8_t* original_copy = (uint8_t*)malloc(original_size);
    ASSERT_NE(original_copy, nullptr);
    memcpy(original_copy, original_data, original_size);
    
    // Deserialize
    struct MSG* deserialized_msg = deserialize_msg(original_data, original_size);
    ASSERT_NE(deserialized_msg, nullptr);
    
    // Second serialization
    size_t second_size;
    uint8_t* second_data = serialize_msg(deserialized_msg, &second_size);
    ASSERT_NE(second_data, nullptr);
    
    // The sizes should match
    EXPECT_EQ(original_size, second_size);
    
    // The contents should be identical
    EXPECT_EQ(memcmp(original_copy, second_data, original_size), 0);
    
    // Cleanup
    free(original_data);
    free(original_copy);
    free(second_data);
    free_msg(deserialized_msg);
}

int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
