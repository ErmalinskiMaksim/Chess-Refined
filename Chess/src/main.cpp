#include "RTWgui/RTWgui.h"
#include <iostream>

int main() {
    int exit_val = EXIT_SUCCESS;

    try {
        RTWgui::run();
    } catch (const std::runtime_error &e) {
        std::cerr << e.what() << std::endl;
        exit_val = EXIT_FAILURE;
    }

    return exit_val;
}
