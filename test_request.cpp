// File: test_request.cpp
// Description: Implementation of a client application that sends a request to a server
//              for a specific file and processes the server's response. The response
//              includes a FileMessage containing the requested file's data.
// Author: Logan Scheetz
// Date: 5/12/25

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "protocol.hpp"  // For RequestMessage, FileMessage, xor42

int main() {
    // Server connection details
    const char* hostname = "127.0.0.1";  // Server hostname or IP address
    int port = 8081;                     // Server port
    const char* filename = "sample.txt"; // File to request from the server

    // 1. Build RequestMessage
    // Create a RequestMessage object for the specified file name.
    RequestMessage req(filename);

    // 2. Serialize & encrypt
    // Convert the RequestMessage into a serialized byte buffer and encrypt it.
    auto serialized = req.serialize();
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
    std::cout << "Connected to server for request.\n";

    // 3. Send RequestMessage
    // Send the encrypted RequestMessage to the server.
    send(sock, encrypted.data(), encrypted.size(), 0);

    // Signal the server that the client has finished sending data.
    shutdown(sock, SHUT_WR);

    // 4. Receive FileMessage
    // Read the response from the server into a buffer.
    std::vector<uint8_t> resp_buf; // Buffer to hold the response
    uint8_t tmp[4096];             // Temporary buffer for reading chunks
    ssize_t n;
    while ((n = recv(sock, tmp, sizeof(tmp), 0)) > 0) {
        resp_buf.insert(resp_buf.end(), tmp, tmp + n); // Append received data to the buffer
    }
    close(sock); // Close the socket after receiving the response

    if (n < 0) {
        perror("recv");
        return 1; // Exit if receiving data fails
    }

    std::cout << "Received " << resp_buf.size() << " bytes encrypted back\n";

    // 5. Decrypt & deserialize FileMessage
    // Decrypt the response and attempt to parse it as a FileMessage.
    auto dec = xor42(resp_buf);
    try {
        auto fm = FileMessage::deserialize(dec); // Parse the response
        std::cout << "File name: " << fm.name << "\n"; // Display the file name
        std::cout << "Data (as string):\n";
        std::cout << std::string(fm.data.begin(), fm.data.end()); // Display the file content
    } catch (const std::exception &e) {
        // Handle any errors during deserialization
        std::cerr << "Failed to parse FileMessage: " << e.what() << "\n";
        return 1;
    }

    return 0;
}