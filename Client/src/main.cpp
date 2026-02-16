#include "RTWgui/RTWgui.h"
#include "ChessClient.h"
#include <print>

int main() {
    ChessClient::get().init();
    ChessClient::get().connect("/tmp/chess-refined.sock");

    try {
        RTWgui::run();
    } catch (const std::runtime_error &e) {
        std::println("RTWgui error: {}", e.what());
    }

    ChessClient::get().requestShutDown();
}
