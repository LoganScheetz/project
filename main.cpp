// File: server.cpp
// Description: Implementation of a simple file server application using sockets.
//              The server supports file storage and retrieval with serialization,
//              encryption, and persistence functionality.
// Author: Logan Scheetz
// Date: 5/12/25

#include "protocol.hpp"   // Include for FileMessage, StatusMessage, RequestMessage, xor42
#include "pack109.hpp"    // Include for KVMap and serialization
#include "hashmap.hpp"    // Include for the FileServerMap class

#include <csignal>        // Signal handling
#include <fstream>        // File I/O
#include <iostream>
#include <string>
#include <vector>
#include <cstring>        // For strcmp
#include <unistd.h>       // For close and other POSIX functions
#include <sys/socket.h>   // For socket-related functions
#include <netinet/in.h>   // For sockaddr_in
#include <arpa/inet.h>    // For inet_pton

constexpr int DEFAULT_PORT = 8081;  // Default port for the server
constexpr int BUFFER_SIZE = 65535; // Maximum buffer size for communication

// Globals for persistence
static FileServerMap *g_store = nullptr;  // Global pointer to the in-memory file store
static std::string    g_persist_file;    // Path to the persistence file

// Function: handle_sigint
// Purpose: Handles the SIGINT signal (Ctrl+C) to ensure data persistence before exiting.
// Parameters:
//   - signal: The signal number (unused in this implementation).
void handle_sigint(int) {
    if (g_store && !g_persist_file.empty()) {
        try {
            // Convert in-memory data to a serialized map
            KVMap out;
            for (const auto &kv : g_store->entries()) {
                out[kv.first] = pack109::serialize(kv.second);
            }
            auto bytes = pack109::serialize_map(out);

            // Write serialized data to the persistence file
            std::ofstream ofs(g_persist_file, std::ios::binary | std::ios::trunc);
            if (!ofs) {
                throw std::runtime_error("Cannot open file for writing");
            }
            ofs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
            if (!ofs) {
                throw std::runtime_error("Error while writing to file");
            }
            std::cout << "\nPersisted " << out.size()
                      << " files to " << g_persist_file << "\n";
        } catch (const std::exception &e) {
            std::cerr << "\nERROR: Failed to persist to '"
                      << g_persist_file << "': " << e.what()
                      << "\nExiting immediately.\n";
            std::_Exit(1); // Exit without cleanup
        }
    }
    std::_Exit(0); // Exit without cleanup
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    std::string bind_ip = "0.0.0.0";  // Default IP to bind the server
    int port = DEFAULT_PORT;          // Default port
    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--hostname") == 0 || strcmp(argv[i], "-h") == 0) {
            // Parse hostname argument in the format IP:PORT
            if (i + 1 < argc) {
                std::string arg = argv[++i];
                auto colon = arg.find(':');
                if (colon != std::string::npos) {
                    bind_ip = arg.substr(0, colon);
                    port    = std::stoi(arg.substr(colon + 1));
                } else {
                    std::cerr << "Invalid hostname format, use IP:PORT\n";
                    return 1;
                }
            }
        } else if (strcmp(argv[i], "--persist") == 0 || strcmp(argv[i], "-p") == 0) {
            // Parse persistence file argument
            if (i + 1 < argc) {
                g_persist_file = argv[++i];
            }
        }
    }

    // Setup signal handler for SIGINT
    std::signal(SIGINT, handle_sigint);

    // Create server socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("socket");
        return 1;
    }

    // Configure server address
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_port   = htons(port);
    if (inet_pton(AF_INET, bind_ip.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "Bad IP address: " << bind_ip << "\n";
        return 1;
    }

    // Bind server socket to the address
    if (bind(server_fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
        perror("bind");
        return 1;
    }

    // Start listening for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("listen");
        return 1;
    }

    std::cout << "Listening on " << bind_ip << ":" << port << std::endl;

    // Initialize in-memory file store
    FileServerMap store;
    g_store = &store;

    // Load persistence file if specified
    if (!g_persist_file.empty()) {
        std::ifstream testifs(g_persist_file);
        if (!testifs) {
            std::cout << "Persist file not found (" << g_persist_file
                      << "), starting with empty store." << std::endl;
        } else {
            std::ifstream ifs(g_persist_file, std::ios::binary);
            std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), {});
            try {
                KVMap disk_map = pack109::deserialize_map(buf);
                for (auto &kv : disk_map) {
                    auto data = pack109::deserialize_vec_u8(kv.second);
                    store.insert(kv.first, data);
                }
                std::cout << "Loaded " << disk_map.size()
                          << " files from " << g_persist_file << std::endl;
            } catch (const std::exception &e) {
                std::cerr << "ERROR: Failed to parse persist file '"
                          << g_persist_file << "': " << e.what() << std::endl;
                return 1;
            }
        }
    }

    // Main server loop: accept and handle client connections
    while (true) {
        std::cout << "Waiting for connection..." << std::endl;
        int client_fd = accept(server_fd, nullptr, nullptr);
        if (client_fd < 0) {
            perror("accept");
            continue;
        }
        std::cout << "Client connected." << std::endl;

        // Communicate with the client
        while (true) {
            std::vector<uint8_t> buf(BUFFER_SIZE);
            ssize_t n = recv(client_fd, buf.data(), buf.size(), 0);
            if (n <= 0) break;  // Client closed connection or error
            buf.resize(n);

            auto decrypted = xor42(buf);

            // 1) Try RequestMessage first
            try {
                auto rm = RequestMessage::deserialize(decrypted);
                try {
                    auto data = store.get(rm.name);
                    FileMessage resp(rm.name, data);
                    auto out = xor42(resp.serialize());
                    send(client_fd, out.data(), out.size(), 0);
                } catch (const std::exception &) {
                    StatusMessage resp(false, std::string("Not found: ") + rm.name);
                    auto out = xor42(resp.serialize());
                    send(client_fd, out.data(), out.size(), 0);
                }
                continue;
            } catch (const std::exception &) {}

            // 2) Try FileMessage
            try {
                auto fm = FileMessage::deserialize(decrypted);
                bool existed = store.insert(fm.name, fm.data);
                StatusMessage resp(true, existed ? "Replaced" : "Stored");
                auto out = xor42(resp.serialize());
                send(client_fd, out.data(), out.size(), 0);
                continue;
            } catch (const std::exception &) {}

            // 3) Invalid message
            {
                StatusMessage resp(false, "Invalid message");
                auto out = xor42(resp.serialize());
                send(client_fd, out.data(), out.size(), 0);
            }
        }

        close(client_fd);
        std::cout << "Client disconnected." << std::endl;
    }

    close(server_fd);
    return 0;
}