#include <filesystem>
#include "ChessServer.h"

int main() {
    try {
        constexpr std::string_view socketPath = "/tmp/chess-refined.sock";

        // Remove leftover socket file (important!)
        std::filesystem::remove(socketPath);

        ChessServer server{socketPath}; 
        server.doAccept();
        server.run();

        std::filesystem::remove(socketPath);
    }
    catch (const std::exception& e) {
        std::println("[SVR]: FATAL ERROR: {}", e.what());
    }
}
