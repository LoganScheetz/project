#include <iostream>
#include <vector>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "protocol.hpp"  // Include for FileMessage and xor42
#include "pack109.hpp"   // Include for Bytes type

// Define buffer size
#define BUFFER_SIZE 4096

int main(int argc, char *argv[]) {
    const char *hostname = "127.0.0.1";
    int port = 8081;

    // Create socket
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Define server address
    sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    inet_pton(AF_INET, hostname, &server_addr.sin_addr);

    // Connect to server
    if (connect(sock, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("Connection failed");
        close(sock);
        return 1;
    }

    std::cout << "Connected to server.\n";

    // Send a request (modify as needed for your protocol)
    const char *request = "Sample request data";
    send(sock, request, strlen(request), 0);

    // Receive response
    std::vector<uint8_t> response;
    uint8_t buffer[BUFFER_SIZE];
    ssize_t bytes_received;

    while ((bytes_received = recv(sock, buffer, BUFFER_SIZE, 0)) > 0) {
        response.insert(response.end(), buffer, buffer + bytes_received);
    }

    if (bytes_received < 0) {
        perror("Receive failed");
        close(sock);
        return 1;
    }

    std::cout << "Received " << response.size() << " bytes from server.\n";

    // Print the raw response (for debugging)
    std::cout << "Encrypted response bytes: ";
    for (uint8_t b : response) {
        std::cout << std::hex << (int)b << " ";
    }
    std::cout << std::endl;

    // Close socket
    close(sock);

    // Decrypt and process the response
    Bytes decrypted = xor42(response); // Use your `xor42` function
    std::cout << "Decrypted response bytes: ";
    for (uint8_t b : decrypted) {
        std::cout << std::hex << (int)b << " ";
    }
    std::cout << std::endl;

    // Deserialize (use your deserialization function)
    try {
        auto message = FileMessage::deserialize(decrypted);
        std::cout << "File name: " << message.name << "\n";
        std::cout << "File data size: " << message.data.size() << " bytes\n";
    } catch (const std::runtime_error &e) {
        std::cerr << "Parse error: " << e.what() << "\n";
    }

    return 0;
}