// File: test_client.cpp
// Description: Implementation of a client application for communicating with a server
//              using the defined protocol. The client sends a file message to the server,
//              receives a response, and processes the response using encryption and serialization.
// Author: Logan Scheetz
// Date: 5/12/25

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "protocol.hpp"  // for FileMessage, RequestMessage, StatusMessage, xor42
#include "pack109.hpp"   // for serialize_map, Bytes

int main(int argc, char *argv[]) {
    // Default values for server connection and file payload
    const char *hostname = "127.0.0.1";  // Server hostname or IP address
    int port = 8081;                     // Server port
    const char *filename = "sample.txt"; // Name of the file to send

    // 1. Prepare a small file payload in-memory
    // Simulating file content as a vector of bytes.
    std::vector<uint8_t> file_data = {'H', 'e', 'l', 'l', 'o', '!', '\n'};

    // 2. Build FileMessage
    // Create a FileMessage object with the file name and data.
    FileMessage fm(filename, file_data);

    // 3. Serialize
    // Convert the FileMessage into a serialized byte buffer.
    auto serialized = fm.serialize();

    // 4. Encrypt
    // Encrypt the serialized data using XOR-42 encryption.
    auto encrypted = xor42(serialized);

    // --- Socket setup ---
    // Create a socket for TCP communication.
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1; // Exit if socket creation fails
    }

    // Configure the server address
    sockaddr_in server_addr{};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port   = htons(port); // Convert port to network byte order
    inet_pton(AF_INET, hostname, &server_addr.sin_addr); // Convert hostname to binary form

    // Connect to the server
    if (connect(sock, (sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        perror("connect");
        close(sock);
        return 1; // Exit if connection fails
    }
    std::cout << "Connected!\n";

    // 5. Send FileMessage
    // Send the encrypted FileMessage to the server.
    send(sock, encrypted.data(), encrypted.size(), 0);

    // 5a. Signal end of write to server
    // Indicate that the client has finished sending data.
    shutdown(sock, SHUT_WR);

    // 6. Receive response
    // Read the response from the server into a buffer.
    std::vector<uint8_t> resp_buf;
    uint8_t tmp[4096];
    ssize_t n;
    while ((n = recv(sock, tmp, sizeof(tmp), 0)) > 0) {
        resp_buf.insert(resp_buf.end(), tmp, tmp + n); // Append received data to response buffer
    }
    close(sock); // Close the socket after receiving the response

    if (n < 0) {
        perror("recv");
        return 1; // Exit if receiving data fails
    }

    std::cout << "Got " << resp_buf.size() << " bytes encrypted back\n";

    // 7. Decrypt and deserialize StatusMessage
    // Decrypt the response and attempt to parse it as a StatusMessage.
    auto resp_dec = xor42(resp_buf);
    try {
        auto status = StatusMessage::deserialize(resp_dec);
        std::cout << "Server responded: " 
                  << (status.ok ? "OK" : "ERROR")
                  << " – " << status.message << "\n";
    } catch (const std::exception &e) {
        std::cerr << "Failed to parse status: " << e.what() << "\n";
    }

    return 0;
}