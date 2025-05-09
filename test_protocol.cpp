#include "protocol.hpp"
#include <cassert>
#include <iostream>

int main() {
    using Bytes = std::vector<uint8_t>;

    // Test XOR-42 invertibility
    Bytes original{1, 42, 100, 255};
    Bytes encrypted = xor42(original);
    assert(encrypted != original && "xor42 should modify the data");
    Bytes decrypted = xor42(encrypted);
    assert(decrypted == original && "xor42 applied twice should restore original data");

    std::cout << "XOR-42 helper tests passed." << std::endl;
    return 0;
}
