// File: test_protocol.cpp
// Description: Unit tests for the protocol classes and XOR-42 helper.
//              Includes tests for FileMessage, RequestMessage, StatusMessage,
//              and the xor42 encryption utility.
// Author: Logan Scheetz
// Date: 5/12/25

#include <iostream>
#include <vector>
#include <cassert>

#include "protocol.hpp"  // FileMessage, RequestMessage, StatusMessage, xor42
#include "pack109.hpp"   // Bytes alias

// Helper to check round-trip encryption and decryption using xor42
// Function: test_xor42
// Purpose: Ensures that data encrypted and then decrypted with xor42 matches the original data.
void test_xor42() {
    std::vector<uint8_t> data = {1, 2, 3, 4, 5}; // Original data
    auto enc = xor42(data);                      // Encrypt the data
    auto dec = xor42(enc);                       // Decrypt the encrypted data
    assert(dec == data);                         // Ensure the decrypted data matches the original
    std::cout << "[ PASS ] XOR-42 helper tests passed.\n";
}

// Test FileMessage serialization and deserialization
// Function: test_file_message
// Purpose: Verifies the FileMessage can be correctly serialized and deserialized.
void test_file_message() {
    std::string name = "foo.txt";                       // File name
    std::vector<uint8_t> payload = {'H', 'e', 'l', 'l', 'o'}; // File content
    FileMessage fm1(name, payload);                     // Create a FileMessage

    auto ser = fm1.serialize();                         // Serialize the FileMessage
    auto fm2 = FileMessage::deserialize(ser);           // Deserialize it back

    assert(fm2.name == name);                           // Check the file name
    assert(fm2.data == payload);                        // Check the file content
    std::cout << "[ PASS ] FileMessage serialize/deserialize\n";
}

// Test RequestMessage serialization and deserialization
// Function: test_request_message
// Purpose: Verifies the RequestMessage can be correctly serialized and deserialized.
void test_request_message() {
    std::string name = "bar.dat";                       // Requested file name
    RequestMessage rm1(name);                           // Create a RequestMessage

    auto ser = rm1.serialize();                         // Serialize the RequestMessage
    auto rm2 = RequestMessage::deserialize(ser);        // Deserialize it back

    assert(rm2.name == name);                           // Check the requested file name
    std::cout << "[ PASS ] RequestMessage serialize/deserialize\n";
}

// Test StatusMessage serialization and deserialization
// Function: test_status_message
// Purpose: Verifies the StatusMessage can be correctly serialized and deserialized.
void test_status_message() {
    // Test case 1: StatusMessage with success
    {
        StatusMessage ok(true, "All good");             // Create a success StatusMessage
        auto ser = ok.serialize();                      // Serialize the StatusMessage
        auto st = StatusMessage::deserialize(ser);      // Deserialize it back
        assert(st.ok == true);                          // Check the success flag
        assert(st.message == "All good");               // Check the message
    }

    // Test case 2: StatusMessage with error
    {
        StatusMessage err(false, "Not found");          // Create an error StatusMessage
        auto ser = err.serialize();                     // Serialize the StatusMessage
        auto st = StatusMessage::deserialize(ser);      // Deserialize it back
        assert(st.ok == false);                         // Check the error flag
        assert(st.message == "Not found");              // Check the message
    }

    std::cout << "[ PASS ] StatusMessage serialize/deserialize\n";
}

// Entry point for the test suite
// Function: main
// Purpose: Runs all the unit tests for the protocol classes and helper functions.
int main() {
    test_xor42();               // Test the XOR-42 encryption utility
    test_file_message();        // Test FileMessage serialization/deserialization
    test_request_message();     // Test RequestMessage serialization/deserialization
    test_status_message();      // Test StatusMessage serialization/deserialization
    std::cout << "All protocol tests passed!\n";
    return 0;
}