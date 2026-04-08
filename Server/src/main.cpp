#include <print>
#include "ChessServer.h"

int main() {
    try {
        ChessServer server{}; 
        server.doAccept();
        server.run();
    }
    catch (const std::exception& e) {
        std::println("[SVR]: FATAL ERROR: {}", e.what());
    }
}
