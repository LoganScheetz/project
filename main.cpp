// src/main.cpp
#include "protocol.hpp"
#include "pack109.hpp"
#include "hashmap.hpp"

#include <csignal>   // signal handling
#include <fstream>   // file I/O
#include <iostream>
#include <string>
#include <vector>
#include <cstring>   // strcmp
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

constexpr int DEFAULT_PORT = 8081;
constexpr int BUFFER_SIZE = 65535;

// Globals for persistence
static FileServerMap *g_store = nullptr;
static std::string    g_persist_file;

void handle_sigint(int) {
    if (g_store && !g_persist_file.empty()) {
        // Persist in-memory map to disk
        pack109::KVMap out;
        for (const auto &kv : g_store->entries()) {
            out[kv.first] = pack109::serialize(kv.second);
        }
        auto bytes = pack109::serialize_map(out);
        std::ofstream ofs(g_persist_file, std::ios::binary);
        ofs.write(reinterpret_cast<const char*>(bytes.data()), bytes.size());
        std::cout << "\nPersisted " << out.size() << " files to " << g_persist_file << "\n";
    }
    std::_Exit(0);
}

int main(int argc, char *argv[]) {
    std::string bind_ip = "0.0.0.0";
    int port = DEFAULT_PORT;
    std::string persist_file;

    // Parse flags
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--hostname") == 0 || std::strcmp(argv[i], "-h") == 0) {
            if (i + 1 < argc) {
                std::string host = argv[++i];
                auto pos = host.find(':');
                if (pos == std::string::npos) { std::cerr << "Usage: --hostname ip:port\n"; return 1; }
                bind_ip = host.substr(0, pos);
                port = std::stoi(host.substr(pos+1));
            } else { std::cerr << "Error: no value for " << argv[i] << "\n"; return 1; }
        } else if (std::strcmp(argv[i], "--persist") == 0 || std::strcmp(argv[i], "-p") == 0) {
            if (i + 1 < argc) { persist_file = argv[++i]; }
            else { std::cerr << "Error: no persist file provided\n"; return 1; }
        }
    }

    // Setup socket
    int server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) { perror("socket"); return 1; }

    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    if (inet_pton(AF_INET, bind_ip.c_str(), &addr.sin_addr) != 1) {
        std::cerr << "Invalid bind address\n"; return 1;
    }
    addr.sin_port = htons(port);

    if (bind(server_fd, (sockaddr*)&addr, sizeof(addr)) < 0) { perror("bind"); return 1; }
    if (listen(server_fd, 1) < 0) { perror("listen"); return 1; }

    std::cout << "Listening on " << bind_ip << ":" << port << std::endl;

    FileServerMap store;

    // Load persistence
    if (!persist_file.empty()) {
        std::ifstream ifs(persist_file, std::ios::binary);
        if (ifs) {
            std::vector<uint8_t> buf((std::istreambuf_iterator<char>(ifs)), {});
            pack109::KVMap disk_map = pack109::deserialize_map(buf);
            for (auto &kv : disk_map) {
                auto data = pack109::deserialize_vec_u8(kv.second);
                store.insert(kv.first, data);
            }
            std::cout << "Loaded " << disk_map.size() << " files from " << persist_file << std::endl;
        }
    }

    // Setup persistence handler
    g_store = &store;
    g_persist_file = persist_file;
    std::signal(SIGINT, handle_sigint);

    // Accept connections
    int client_fd = accept(server_fd, nullptr, nullptr);
    if (client_fd < 0) { perror("accept"); return 1; }

    while (true) {
        std::vector<uint8_t> buf(BUFFER_SIZE);
        ssize_t n = recv(client_fd, buf.data(), buf.size(), 0);
        if (n <= 0) break;
        buf.resize(n);

        auto decrypted = xor42(buf);
        try {
            auto fm = FileMessage::deserialize(decrypted);
            bool existed = store.insert(fm.name, fm.data);
            StatusMessage resp(true, existed ? "Replaced" : "Stored");
            auto out = resp.serialize();
            send(client_fd, out.data(), out.size(), 0);
            continue;
        } catch(...) {}

        try {
            auto rm = RequestMessage::deserialize(decrypted);
            auto data = store.get(rm.name);
            FileMessage resp(rm.name, data);
            auto out = resp.serialize();
            send(client_fd, out.data(), out.size(), 0);
            continue;
        } catch(...) {}

        StatusMessage resp(false, "Invalid message");
        auto out = resp.serialize();
        send(client_fd, out.data(), out.size(), 0);
    }

    close(client_fd);
    close(server_fd);
    return 0;
}
