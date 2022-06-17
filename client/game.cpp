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

// Globals
static SOCKET server_sock = NULL;

static Room room = {};
static nlohmann::json roomJSON;
static int done_setup = 0;
static std::mutex mu;

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

Element VillageString(int vill, int doc, int cop, int esc, int arms) {
    return text(
        "Villager: " + std::to_string(vill) + " | " +
        "Doctor: " + std::to_string(doc) + " | " +
        "Cop: " + std::to_string(cop) + " | " +
        "Escort: " + std::to_string(esc) + " | " +
        "Arms Dealer: " + std::to_string(arms)
    );
}

Element MafiaString(int godf, int mafia) {
    return text(
        "Godfather: " + std::to_string(godf) + " | " +
        "Mafioso: " + std::to_string(mafia)
    );
}

Element IndependentString(int jest) {
    return text(
        "Jester: " + std::to_string(jest)
    );
}

void room_setup(SOCKET ConnectSocket) {
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
    int villager_count = 0;
    int doctor_count = 0;
    int cop_count = 0;
    int escort_count = 0;
    int armsdealer_count = 0;
    // -- Mafia
    int godfather_count = 0;
    int mafioso_count = 0;
    // -- Independent
    int jester_count = 0;

    auto village_tab_component = Container::Vertical(
        {
        Slider("Villager", &villager_count, 0, 5, 1),
        Slider("Doctor", &doctor_count, 0, 5, 1),
        Slider("Cop", &cop_count, 0, 5, 1),
        Slider("Escort", &escort_count, 0, 5, 1),
        Slider("Arms dealer", &armsdealer_count, 0, 5, 1),
        }
    );

    auto mafia_tab_component = Container::Vertical(
        {
        Slider("Godfather", &godfather_count, 0, 5, 1),
        Slider("Mafioso", &mafioso_count, 0, 5, 1),
        }
    );

    auto independent_tab_component = Container::Vertical(
        {
        Slider("Jester", &jester_count, 0, 5, 1),
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
                text("Room setup! You are the leader.") | ftxui::borderRounded | flex,
            }) | bold,
            separatorDouble(),
            hbox(text("Day length (in mins): "), text(std::to_string(day_length))),
            slider_day->Render(),
            hbox(text("Night length (in mins): "), text(std::to_string(night_length))),
            slider_night->Render(),
            separatorDouble(),
            checkboxes->Render(),
            separatorDouble(),

            role_tab_toggle->Render(),
            separatorDouble(),
            role_tab_container->Render(),
            separator(),
            VillageString(villager_count, doctor_count, cop_count, escort_count, armsdealer_count),
            MafiaString(godfather_count, mafioso_count),
            IndependentString(jester_count),

            separatorDouble(),
            setup_buttons->Render(),
            }) | borderHeavy;
        });
    setup_screen.Loop(setup_component);

    /////////////////////////////////////////////////////////////////
    // Json

    Settings s {
        day_length,
        night_length,
        last_will_selected,
        no_reveal_selected,
        day_start_selected,
    };

    Roles r {
        // -- Village
        villager_count,
        doctor_count,
        cop_count,
        escort_count,
        armsdealer_count,
        // -- Mafia
        godfather_count,
        mafioso_count,
        // -- Independent
        jester_count,
    };

    room.roles = r;
    room.settings = s;

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

    nlohmann::json room_json = {
        {"settings", settings_json}, {"roles", roles_json}
    };

    roomJSON = room_json;

    /////////////////////////////////////////////////////////////////
    // Send JSON

    auto room_json_str = room_json.dump();
    std::cout << room_json_str << std::endl;

    if (send_data(ConnectSocket, room_json_str.data()) <= 0)
        return;
}

void recieve_setup(SOCKET ConnectSocket) {
    std::vector<char> room_buf(1024);
    std::string room_json_str;
    int iResult = 0;
    while (iResult <= 0) {
        iResult = recieve_data(ConnectSocket, room_buf);
        if (iResult == 0 || iResult == -1) {
            std::cout << "Lost connection to server!" << std::endl;
            return;
        }
    }
    room_json_str.append(room_buf.cbegin(), room_buf.cend());

    json room_json = json::parse(room_json_str);
    roomJSON = room_json;

    Roles roles = {
        room_json["roles"]["villager"],
        room_json["roles"]["doctor"],
        room_json["roles"]["cop"],
        room_json["roles"]["escort"],
        room_json["roles"]["armsdealer"],
        room_json["roles"]["godfather"],
        room_json["roles"]["mafioso"],
        room_json["roles"]["jester"],
    };

    Settings settings = {
        room_json["settings"]["day_length"],
        room_json["settings"]["night_length"],
        room_json["settings"]["last_will"],
        room_json["settings"]["no_reveal"],
        room_json["settings"]["day_start"],
    };

    std::cout << room_json_str << std::endl;
    
    room.roles = roles;
    room.settings = settings;

    done_setup = 1;
}

int wait_for_setup() {
    using namespace ftxui;
    using namespace std::chrono_literals;

    std::string reset_position;
    for (int index = 0; index < 1000; ++index) {
        // Check if setup recieved
        if (done_setup > 0) {
            return done_setup;
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

void send_chat(std::string msg_buf) {
    std::string msg = "0#";
    msg += msg_buf;
    send_data(server_sock, msg.data());
}

void start_game() {
    // Clear screen
    system("cls");

    // Full screen
    system("mode 650");
    ShowWindow(GetConsoleWindow(), SW_MAXIMIZE);

    auto screen = ScreenInteractive::TerminalOutput();

    const std::vector<std::string> entries = {
        "Player 1",
        "Player 2",
        "Player 3",
        "Player 4",
        "Player 5",
        "Player 6",
        "Player 7",
    };

    // -- Action target ---------------------------------------

    int selected = 0;

    auto target_select_layout = Container::Vertical({
        Radiobox(&entries, &selected)
        });

    std::string action_name = "Kill";

    // Action and player selection
    auto targets_list = Renderer(target_select_layout, [&] {
        return window(text("Action:"), vbox({text(action_name), separator(), target_select_layout->Render() | vscroll_indicator | frame |
            size(HEIGHT, LESS_THAN, 10)  }) | border);
        });

    // Last will input lines + window
    InputOption lastwill_option;
    lastwill_option.on_enter = screen.ExitLoopClosure();
    std::vector<std::string> lastwill_lines(6);
    Component input_lastwill_1 = Input(&lastwill_lines[0], "", lastwill_option); // input option for debugging
    Component input_lastwill_2 = Input(&lastwill_lines[1], "");
    Component input_lastwill_3 = Input(&lastwill_lines[2], "");
    Component input_lastwill_4 = Input(&lastwill_lines[3], "");
    Component input_lastwill_5 = Input(&lastwill_lines[4], "");
    Component input_lastwill_6 = Input(&lastwill_lines[5], "");

    auto input_lastwill = Container::Vertical({
        input_lastwill_1,
        input_lastwill_2,
        input_lastwill_3,
        input_lastwill_4,
        input_lastwill_5,
        input_lastwill_6,
        });

    //auto lastwill_window = window(text("Last will"), input_lastwill->Render());

    // -- Players list --------------------------------------------

    int player_alive_selected = 0;
    auto players_alive_menu = Menu(&entries, &player_alive_selected);

    int player_dead_selected = 0;
    auto players_dead_menu = Menu(&entries, &player_dead_selected);

    // -- Chat ----------------------------------------------------

    std::string chat_input_buf;
    InputOption chat_input_option;

    chat_input_option.on_enter = [&] {
        send_chat(chat_input_buf);
        chat_input_buf = "";
    };
    Component chat_input = Input(&chat_input_buf, "", chat_input_option);

    // -- Layouts -------------------------------------------------

    auto right_side = Container::Vertical({
        targets_list,
        input_lastwill,
        });

    auto left_side = Container::Vertical({
        players_alive_menu,
        players_dead_menu,
        });

    auto chat = Container::Vertical({
        //
        chat_input
        });

    auto game_layout = Container::Horizontal({
        left_side,
        chat,
        right_side,
        });

    auto page_component = Renderer(game_layout, [&] {
        return hbox({
            // -- Left side --
            vbox({
                window(text("Alive players:"), vbox({ players_alive_menu->Render() | vscroll_indicator | frame |
                    size(HEIGHT, EQUAL, 15)})),
                window(text("Dead players:"), vbox({ players_dead_menu->Render() | vscroll_indicator | frame |
                    size(HEIGHT, EQUAL, 15)})),
            }) | size(WIDTH, EQUAL, 30),

            filler(),

            // -- Center --
            vbox({
                text("Chat:"),
                separator(),
                vbox({

                }) | size(HEIGHT, EQUAL, 28),
                separator(),
                hbox(text("Your msg: ") | underlined, chat_input->Render()) | border,
            }) | size(WIDTH, EQUAL, 100) | border,

            filler(),

            // -- Right side --
            vbox({
                targets_list->Render() | size(HEIGHT, EQUAL, 15),
                vbox({
                    window(text("Last will"), input_lastwill->Render())
                    }) | size(WIDTH, EQUAL, 60),
            }) | size(HEIGHT, EQUAL, 30),
        }) | borderHeavy;
    });

    screen.Loop(page_component);

    while(1){}
}

void handle_player () {
	SOCKET ConnectSocket = NULL;
    do {
        ConnectSocket = connect();
        server_sock = ConnectSocket;
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
                   hbox({
                        text("Hello "),
                        text(name_buf) | underlined,
                        text(". Welcome to Mafioso!"),
                   }) | bold,
                   separator(),
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

    //std::cout << "Clients count = " << clients_count << std::endl;

    /////////////////////////////////////////////////////////////////

    if (clients_count == 1) {
        room_setup(ConnectSocket);
        start_game();
    }
    else if (done_setup == 0) {
        auto future = std::async(std::launch::async, recieve_setup, std::ref(ConnectSocket));
        if (wait_for_setup()) {
            start_game();
            while (1) {};
            return;
        }
    }
    /*else if (done_setup == 1) {
        recieve_setup(ConnectSocket, done_setup);
    }*/
}