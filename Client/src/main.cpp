#include "RTWgui/RTWgui.h"
#include "ChessClient.h"
#include <print>

int main(int argc, char* argv[]) {
    constexpr std::string_view fallbackIP = "127.0.0.1";

    ChessClient client;
    client.init();
    client.connect((argc > 1) ? argv[1] : fallbackIP);

    try {
        RTWgui::run();
    } catch (const std::runtime_error &e) {
        std::println("RTWgui error: {}", e.what());
    }
}
