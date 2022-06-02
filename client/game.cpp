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

// FTXUI - Input
#include "ftxui/component/captured_mouse.hpp"  // for ftxui
#include "ftxui/component/component.hpp"       // for Input, Renderer, Vertical
#include "ftxui/component/component_base.hpp"  // for ComponentBase
#include "ftxui/component/component_options.hpp"  // for InputOption
#include "ftxui/component/screen_interactive.hpp"  // for Component, ScreenInteractive
#include "ftxui/dom/elements.hpp"  // for text, hbox, separator, Element, operator|, vbox, border
#include "ftxui/util/ref.hpp"  // for Ref

// Game
#include "connection.h"
#include "../server/room.h"

bool joined = 0;

using namespace ftxui;

Component Wrap(std::string name, Component component) {
    return Renderer(component, [name, component] {
        return hbox({
                text(name) | size(WIDTH, EQUAL, 8),
                separator(),
                component->Render() | xflex,
        }) |
        xflex;
    });
}

void handle_player () {
	SOCKET ConnectSocket;
    do {
        ConnectSocket = connect();
    } while (ConnectSocket == NULL);

    joined = true;
    
	system("cls");

    /////////////////////////////////////////////////////////////////
    // Input and send name

    std::string name_buf;
    auto screen_name = ScreenInteractive::TerminalOutput();
    InputOption option_input;
    option_input.on_enter = screen_name.ExitLoopClosure();
    Component input_name = Input(&name_buf, "your name here", option_input);

    auto component_input = Container::Vertical({
        input_name,
    });

    auto renderer = Renderer(component_input, [&] {
        return vbox({
                   hbox(text("Player name: "), input_name->Render()),
            }) |
            border;
    });
    screen_name.Loop(renderer);

    system("cls");

    if (send_data(ConnectSocket, name_buf.data()) <= 0)
        return;
    
    /////////////////////////////////////////////////////////////////
    // Room
    
    // hbox
    //    Element document =
    //    hbox({
    //        text("Room setup! You are the leader.") | ftxui::border | flex,
    //        });

    //auto screen_top = Screen::Create(
    //    Dimension::Full(),       // Width
    //    Dimension::Fit(document) // Height
    //);
    //ftxui::Render(screen_top, document);
    //screen_top.Print();
    // 
    //std::cout << std::endl;

    /////////////////////////////////////////////////////////////////
    // Room setup

    // -- Settings --------------------------------------------------
     int day_length = 2;
     int night_length = 1;
     bool last_will_selected = false;
     bool no_reveal_selected = false;
     bool day_start_selected = false;
     auto screen_setup = ScreenInteractive::TerminalOutput();

     auto slider_day = Slider("", &day_length, 1, 10, 1);
     auto slider_night = Slider("", &night_length, 1, 10, 1);

     auto checkboxes = Container::Vertical({
            Checkbox("Last will", &last_will_selected),
            Checkbox("No reveal", &no_reveal_selected),
            Checkbox("Day start", &day_start_selected),
         });

     // -- Roles ---------------------------------------------------
     std::vector<std::string> tab_values{
        "Village",
        "Mafia",
        "Independent",
     };
     int role_tab_selected = 0;
     auto role_tab_toggle = Toggle(&tab_values, &role_tab_selected);

     // -- Village
     bool villager_selected = false;
     bool doctor_selected = false;
     bool cop_selected = false;
     bool escort_selected = false;
     bool armsdealer_selected = false;
     // -- Mafia
     bool godfather_selected = false;
     bool mafioso_selected = false;
     // -- Independent
     bool jester_selected = false;

     auto village_tab_component = Container::Vertical(
         {
            Checkbox("Villager", &villager_selected),
            Checkbox("Doctor", &doctor_selected),
            Checkbox("Cop", &cop_selected),
            Checkbox("Escort", &escort_selected),
            Checkbox("Arms dealer", &armsdealer_selected),
         }
     );

     auto mafia_tab_component = Container::Vertical(
         {
            Checkbox("Godfather", &godfather_selected),
            Checkbox("Mafioso", &mafioso_selected),
         }
     );

     auto independent_tab_component = Container::Vertical(
         {
            Checkbox("Jester", &jester_selected),
         }
     );

     auto role_tab_container = Container::Tab(
         {
            village_tab_component,
            mafia_tab_component,
            independent_tab_component,
         }, 
         &role_tab_selected);

     auto setup_buttons = Container::Horizontal(
         {
            Button("Create room", [&] { return; }),
            Button("Disconnect", [&] {disconnect(ConnectSocket); exit(0); }),
         }
     );


     auto layout_setup = Container::Vertical({
         slider_day,
         slider_night,
         checkboxes,
         role_tab_toggle,
         role_tab_container,
         setup_buttons,
     });

     auto component_setup = Renderer(layout_setup, [&] {
         return vbox({
                hbox({
                    text("Room setup! You are the leader.") | ftxui::border | flex,
                }),
                separator(),
                hbox(text("Day length (in mins) "), text(std::to_string(day_length))),
                slider_day->Render(),
                hbox(text("Night length (in mins) "), text(std::to_string(night_length))),
                slider_night->Render(),
                separator(),
                checkboxes->Render(),
                separator(),

                role_tab_toggle->Render(),
                separator(),
                role_tab_container->Render(),

                separator(),
                setup_buttons->Render(),
         }) | border;
     });
     screen_setup.Loop(component_setup);


    /////////////////////////////////////////////////////////////////

    //// Menu
    //auto screen = ScreenInteractive::TerminalOutput();

    //std::vector<std::string> entries = {
    //    "entry 1",
    //    "entry 2",
    //    "entry 3",
    //};
    //int selected = 0;

    //MenuOption option;
    //option.on_enter = screen.ExitLoopClosure();
    //auto menu = Menu(&entries, &selected, &option);
    ////auto menu = Menu(&entries, &selected);



    //// The tree of components. This defines how to navigate using the keyboard.
    //auto buttons = Container::Horizontal({
    //    Button("Create Room", [&] { return; }),
    //    Button("Refresh", [&] { return; }),
    //    Button("Disconnect", [&] {disconnect(ConnectSocket); exit(0); }),
    //    });

    //// -- Layout -----------------------------------------------------------------
    //auto layout = Container::Vertical({
    //    menu,
    //    buttons,
    //    });

    //// Modify the way to render them on screen:
    //auto component = Renderer(layout, [&] {
    //    return vbox({
    //            menu->Render(),
    //            separator(),
    //            buttons->Render(),
    //        });
    //    });

    //screen.Loop(component);
    ////std::cout << selected;
}