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

// JSON
#include "nlohmann/json.hpp"
using json = nlohmann::json;

// general
#include <cstdlib>      // for exit function
#include <future>       // for async

// Game
#include "connection.h"
#include "../server/room.h"

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

void room_setup(SOCKET &ConnectSocket) {
    /////////////////////////////////////////////////////////////////
    // Room setup

    // -- Settings --------------------------------------------------
    int day_length = 2;
    int night_length = 1;
    bool last_will_selected = false;
    bool no_reveal_selected = false;
    bool day_start_selected = false;
    auto setup_screen = ScreenInteractive::TerminalOutput();

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
        Button("Create room", setup_screen.ExitLoopClosure()),
        Button("Disconnect", [&] {disconnect(ConnectSocket); exit(0); }),
        }
    );

    auto setup_layout = Container::Vertical({
        slider_day,
        slider_night,
        checkboxes,
        role_tab_toggle,
        role_tab_container,
        setup_buttons,
        });

    auto setup_component = Renderer(setup_layout, [&] {
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
    setup_screen.Loop(setup_component);

    /////////////////////////////////////////////////////////////////
    // Json

    Settings s{
    day_length,
    night_length,
    last_will_selected,
    no_reveal_selected,
    day_start_selected,
    };

    Roles r{
        // -- Village
    villager_selected,
    doctor_selected,
    cop_selected,
    escort_selected,
    armsdealer_selected,
    // -- Mafia
    godfather_selected,
    mafioso_selected,
    // -- Independent
    jester_selected,
    };

    nlohmann::json roles_json;
    roles_json["villager"] = r.villager;
    roles_json["doctor"] = r.doctor;
    roles_json["cop"] = r.cop;
    roles_json["escort"] = r.escort;
    roles_json["armsdealer"] = r.armsdealer;
    roles_json["godfather"] = r.godfather;
    roles_json["mafioso"] = r.mafioso;
    roles_json["jester"] = r.jester;

    nlohmann::json settings_json;
    settings_json["day_length"] = s.day_length;
    settings_json["night_length"] = s.night_length;
    settings_json["last_will"] = s.last_will;
    settings_json["no_reveal"] = s.no_reveal;
    settings_json["day_start"] = s.day_start;

    /////////////////////////////////////////////////////////////////
    // Send JSON

    auto roles_json_str = roles_json.dump();
    auto settings_json_str = settings_json.dump();
    if (send_data(ConnectSocket, roles_json_str.data()) <= 0)
        return;

    if (send_data(ConnectSocket, settings_json_str.data()) <= 0)
        return;
}

void recieve_setup(SOCKET &ConnectSocket, Roles &r, Settings &s, int &done) {
    std::vector<char> roles_buf(1024);
    std::string roles_json_str;
    int iResult = 0;
    while (iResult <= 0) {
        iResult = recieve_data(ConnectSocket, roles_buf);
        if (iResult == 0 || iResult == -1) {
            std::cout << "Lost connection to server!" << std::endl;
            return;
        }
    }
    roles_json_str.append(roles_buf.cbegin(), roles_buf.cend());

    std::vector<char> settings_buf(1024);
    std::string settings_json_str;
    iResult = 0;
    while (iResult <= 0) {
        iResult = recieve_data(ConnectSocket, settings_buf);
        if (iResult == 0 || iResult == -1) {
            std::cout << "Lost connection to server!" << std::endl;
            return;
        }
    }
    settings_json_str.append(settings_buf.cbegin(), settings_buf.cend());

    json roles_json = json::parse(roles_json_str);
    json settings_json = json::parse(settings_json_str);

    r = {
        roles_json["villager"],
        roles_json["doctor"],
        roles_json["cop"],
        roles_json["escort"],
        roles_json["armsdealer"],
        roles_json["godfather"],
        roles_json["mafioso"],
        roles_json["jester"],
    };

    s = {
        settings_json["day_length"],
        settings_json["night_length"],
        settings_json["last_will"],
        settings_json["no_reveal"],
        settings_json["day_start"],
    };

    std::cout << roles_json_str << std::endl;
    std::cout << settings_json_str << std::endl;

    done = 1;
}

int wait_for_setup(int &done) {
    using namespace ftxui;
    using namespace std::chrono_literals;

    std::string reset_position;
    for (int index = 0; index < 1000; ++index) {
        // Check if setup recieved
        if (done > 0) {
            return done;
        }

        std::vector<Element> entries;
        entries.push_back(
            hbox({
                spinner(20, index) | bold,
                })
        );
        
        auto document = hbox({
            hbox({
                text("Waiting for leader to setup room!") | ftxui::border | flex,
            }),
            vbox(std::move(entries)) | flex,
            }) | flex;
        auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));
        Render(screen, document);
        std::cout << reset_position;
        screen.Print();
        reset_position = screen.ResetPosition();

        std::this_thread::sleep_for(0.1s);
    }
    std::cout << std::endl;
    return 0;
}

void handle_player () {
	SOCKET ConnectSocket = NULL;
    do {
        ConnectSocket = connect();
    } while (ConnectSocket == NULL);
    
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
    // Recieve info about room

    int clients_buf;
    int iResult = 0;
    if (iResult <= 0) {
        iResult = recv(ConnectSocket, (char *)&clients_buf, sizeof(clients_buf), 0);
        if (iResult == 0 || iResult == -1) {
            std::cout << "Lost connection to server!" << std::endl;
            return;
        }
    }
    int clients_count = clients_buf;

    std::cout << "Clients count = " << clients_count << std::endl;

    /////////////////////////////////////////////////////////////////

    if (clients_count == 1) {
        room_setup(ConnectSocket);
    }
    else {
        int done = 0;
        Roles r;
        Settings s;
        auto future = std::async(std::launch::async, recieve_setup, std::ref(ConnectSocket), std::ref(r), std::ref(s), std::ref(done));
        wait_for_setup(done);
        std::cout << "Done" << std::endl;
    }
    
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