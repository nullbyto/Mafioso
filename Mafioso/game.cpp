// FTXUI - hbox
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/screen.hpp>
#include <iostream>

#include <stdlib.h>

// FTXUI - Menu
#include <functional>  // for function
#include <iostream>  // for basic_ostream::operator<<, operator<<, endl, basic_ostream, basic_ostream<>::__ostream_type, cout, ostream
#include <string>    // for string, basic_string, allocator
#include <vector>    // for vector

#include "ftxui/component/captured_mouse.hpp"      // for ftxui
#include "ftxui/component/component.hpp"           // for Menu
#include "ftxui/component/component_options.hpp"   // for MenuOption
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive

// FTXUI - Button
#include <memory>  // for shared_ptr, __shared_ptr_access
#include <string>  // for operator+, to_string

#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"  // for Button, Horizontal, Renderer
#include "ftxui/component/component_base.hpp"      // for ComponentBase
#include "ftxui/component/screen_interactive.hpp"  // for ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for separator, gauge, text, Element, operator|, vbox, border

// Game
#include "connection.h"
#include "../server/room.h"

void handle_player () {
	SOCKET ConnectSocket;
    do {
        ConnectSocket = connect();
    } while (ConnectSocket == NULL);

    using namespace ftxui;
	system("cls");
    // hbox
        Element document =
        hbox({
            text("Room List") | ftxui::border | flex,
            });

    auto screen = Screen::Create(
        Dimension::Full(),       // Width
        Dimension::Fit(document) // Height
    );
    Render(screen, document);
    screen.Print();

    std::cout << std::endl;

    /////////////////////////////////////////////////////////////////
    // Recieve rooms
    //std::vector<Room> rooms;
    //auto rooms_str = recieve_data(ConnectSocket);


    /////////////////////////////////////////////////////////////////

    // Menu
    auto screen1 = ScreenInteractive::TerminalOutput();

    std::vector<std::string> entries = {
        "entry 1",
        "entry 2",
        "entry 3",
    };
    int selected = 0;

    MenuOption option;
    option.on_enter = screen1.ExitLoopClosure();
    auto menu = Menu(&entries, &selected, &option);
    //auto menu = Menu(&entries, &selected);



    // The tree of components. This defines how to navigate using the keyboard.
    auto buttons = Container::Horizontal({
        Button("Create Room", [&] { return; }),
        Button("Refresh", [&] { return; }),
        Button("Disconnect", [&] {disconnect(ConnectSocket); exit(0); }),
        });

    // -- Layout -----------------------------------------------------------------
    auto layout = Container::Vertical({
        menu,
        buttons,
        });

    // Modify the way to render them on screen:
    auto component = Renderer(layout, [&] {
        return vbox({
                menu->Render(),
                separator(),
                buttons->Render(),
            });
        });

    screen1.Loop(component);
    //std::cout << selected;
}