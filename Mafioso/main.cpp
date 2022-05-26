// General
#include <functional>  // for function
#include <iostream>  // for basic_ostream::operator<<, operator<<, endl, basic_ostream, basic_ostream<>::__ostream_type, cout, ostream
#include <string>    // for string, basic_string, allocator
#include <vector>    // for vector

// FTXUI
#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

// Game
#include "connection.h"
#include "game.h"

int main(int argc, const char* argv[]) {
    using namespace ftxui;
    auto screen = ScreenInteractive::TerminalOutput();

    std::vector<std::string> entries = {
        "Connect to the game server",
        "Quit",
    };
    int selected = 0;

    MenuOption option;
    option.on_enter = screen.ExitLoopClosure();
    auto menu = Menu(&entries, &selected, &option);

    screen.Loop(menu);

    //std::cout << "Selected element = " << selected << std::endl;


    switch (selected) {
    case 0:
        handle_player();
        break;
    case 1:
        break;
    }
}
