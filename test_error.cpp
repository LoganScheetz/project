// File: test_error.cpp
// Description: Implementation of a client application that sends a request for a non-existent file
//              to the server and processes the server's response. Demonstrates the use of the
//              defined protocol and encryption.
// Author: Logan Scheetz
// Date: 5/12/25

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "protocol.hpp" // For RequestMessage, StatusMessage, xor42

int main() {
    // Server connection details
    const char* hostname = "127.0.0.1";  // Server hostname or IP address
    int port = 8081;                     // Server port
    const char* missing = "no_such_file.txt"; // Name of a file that doesn't exist

    // 1. Create a RequestMessage for the missing file
    RequestMessage req(missing);

    // 2. Serialize and encrypt the RequestMessage
    auto enc = xor42(req.serialize());

    // --- Socket setup ---
    // Create a socket for TCP communication
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return 1; // Exit if socket creation fails
    }

    // Configure the server address
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port); // Convert port to network byte order
    inet_pton(AF_INET, hostname, &addr.sin_addr); // Convert hostname to binary form

    // Connect to the server
    if (connect(sock, (sockaddr*)&addr, sizeof(addr)) < 0) {
        perror("connect");
        return 1; // Exit if connection fails
    }

    // 3. Send the encrypted RequestMessage to the server
    send(sock, enc.data(), enc.size(), 0);

    // 4. Signal end of write to server
    shutdown(sock, SHUT_WR);

    // 5. Receive the server's response
    std::vector<uint8_t> buf; // Buffer to hold the response
    uint8_t tmp[4096];        // Temporary buffer for reading chunks
    ssize_t n;
    while ((n = recv(sock, tmp, sizeof(tmp), 0)) > 0) {
        buf.insert(buf.end(), tmp, tmp + n); // Append received data to the buffer
    }
    close(sock); // Close the socket after receiving the response

    if (n < 0) {
        perror("recv");
        return 1; // Exit if receiving data fails
    }

    // 6. Decrypt and deserialize the response
    auto dec = xor42(buf);
    try {
        auto st = StatusMessage::deserialize(dec); // Parse the response as a StatusMessage

        // Display the server's response
        std::cout << "Status ok=" << st.ok
                  << " msg=\"" << st.message << "\"\n";

        // Return success if the status indicates the file was not found
        return (st.ok == false && st.message.find("Not found") == 0) ? 0 : 1;
    } catch (...) {
        // Handle any errors during deserialization
        std::cerr << "Failed to parse StatusMessage\n";
        return 1;
    }
}