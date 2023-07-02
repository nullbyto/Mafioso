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

// -- JSON
#include "nlohmann/json.hpp"
using json = nlohmann::json;

// -- general
#include <cstdlib>      // for exit function
#include <future>       // for async

// -- Game
#include "connection.h"
#include "../server/room.h"

// #############################################################
// #############################################################

// -- Globals -------------------------------

#define FLAG_FAIL "!#"
#define FLAG_CHAT "0#"
#define FLAG_GAME "1#"
#define FLAG_INFO "2#"
#define FLAG_ROOM "3#"

static SOCKET server_sock = NULL;

static int done_setup = 0;
static int done_name = 0;
static int ddd = 0;
static std::mutex mu;

static int chat_msgs_selected = 0;
static std::vector<std::string> chat_msgs = {};

// Game
static std::string player_name;
static Room room = {};
static nlohmann::json roomJSON;
static std::list<Player> players = {};
static std::vector<std::string> players_list = {};

// -----------------------------------------

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

// #############################################################
// #############################################################

void parse_room_json(std::string room_json_str) {
    if (room_json_str.substr(0, 2) == FLAG_FAIL) {
        done_setup = -1;
        return;
    }
    room_json_str = room_json_str.substr(2, room_json_str.size());

    json room_json;
    try {
         room_json = json::parse(room_json_str);
    }
    catch (json::parse_error& ex) {
        std::cout << "Error parsing room json.\n";
        exit(0);
    }

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

    //std::cout << room_json_str << std::endl;

    room.roles = roles;
    room.settings = settings;

    // Parse players list from json
    for (int i = 0; i < room_json["players"].size(); i++) {
        std::string name = room_json["players"][i]["name"];
        int id = room_json["players"][i]["id"];
        role_code role = room_json["players"][i]["role"];
        Player p;
        p.name = name;
        p.id = id;
        p.role = role;

        // Check if player already exists in the list
        bool found = std::find(players_list.begin(), players_list.end(), name) != players_list.end();
        if (!found) {
            room.players.push_back(p);
            players_list.push_back(p.name);
        }
    }
    players = room.players;
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
    //std::cout << room_json_str << std::endl;

    if (send_data(ConnectSocket, room_json_str.data()) <= 0)
        return;
}

void recieve_setup(SOCKET ConnectSocket) {
    std::vector<char> room_buf(1024);
    std::string room_json_str;
    int iResult = 0;
    iResult = recv(ConnectSocket, room_buf.data(), room_buf.size(), 0);
    if (iResult == 0 || iResult == -1) {
        std::cout << "Lost connection to server!" << std::endl;
        return;
    }

    room_json_str.append(room_buf.cbegin(), room_buf.cend());

    parse_room_json(room_json_str);

    // Signal done setup to the waiting screen which its thread reads
    done_setup = 1;
}

int wait_for_setup() {
    using namespace ftxui;
    using namespace std::chrono_literals;

    std::string reset_position;
    for (int index = 0; index < 100; ++index) {
        // Check if setup recieved
        if (done_setup == 1 || done_setup == -1) {
            return done_setup;
        }

        std::vector<Element> entries;
        entries.push_back(
            hbox({
                spinner(20, index) | bold,
                })
        );

        auto document = vbox({
            hbox({
                hbox({
                    text("Connected! Waiting for the leader to setup room!"),
                }) | ftxui::border,
                vbox(std::move(entries)) | flex,
            }),
            }) | flex;

        auto screen = Screen::Create(Dimension::Full(), Dimension::Fit(document));

        ftxui::Render(screen, document);

        std::cout << reset_position;
        screen.Print();
        reset_position = screen.ResetPosition();

        std::this_thread::sleep_for(0.1s);
    }
    std::cout << std::endl;
    return 0;
}

void send_chat(std::string msg_buf) {
    std::string msg = FLAG_CHAT;
    msg += player_name + ": ";
    msg += msg_buf;
    send_data(server_sock, msg.data());
}

void handle_data() {
    while (1) {
        std::vector<char> data_buf(1024);
        std::string data;
        int iResult = 0;
        
        iResult = recv(server_sock, data_buf.data(), data_buf.size(), 0);
        
        if (iResult == 0 || iResult == -1) {
            std::cout << "Lost connection to server!" << std::endl;
            return;
        }
        data.append(data_buf.cbegin(), data_buf.cend());
        std::string data_raw = data.substr(2, data.size());

        auto prefix = data.substr(0, 2);

        if (prefix == FLAG_CHAT) {
            chat_msgs.push_back(data_raw);
            // Change selected msg in chat to scroll focus to bottom
            chat_msgs_selected = (int)chat_msgs.size() - 1;
        }
        else if (prefix == FLAG_GAME) {
            std::cout << "ma3\n";
        }
        else if (prefix == FLAG_INFO) {
            parse_room_json(data);
            done_setup = 1;
        }
        else if (prefix == FLAG_ROOM) {
            parse_room_json(data);
            done_setup = 1;
        }
    }

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

    // -- Players list --------------------------------------------

    int player_alive_selected = 0;
    auto players_alive_menu = Menu(&players_list, &player_alive_selected);

    int player_dead_selected = 0;
    auto players_dead_menu = Menu(&entries, &player_dead_selected);

    // -- Chat 

    auto chat_component = Renderer([&] {
        Elements elements;
        for (auto& line : chat_msgs) {
            elements.push_back(text(line));
        }
        return vbox(std::move(elements));
        });

    //auto chat_msgs_selected = 0;
    auto chat_msgs_menu = Menu(&chat_msgs, &chat_msgs_selected);

    std::string chat_input_buf;
    InputOption chat_input_option;
    chat_input_option.on_enter = [&] {
        if (chat_input_buf == "") {
            return;
        }
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
        chat_msgs_menu,
        //chat_component,
        chat_input,
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
                    //chat_component->Render() | focusPositionRelative(0, 1) | yframe,
                    chat_msgs_menu->Render() | vscroll_indicator | frame,
                }) | size(HEIGHT, EQUAL, 28),
                separator(),
                hbox(text("Your msg") | underlined, text(": "), chat_input->Render()) | border,
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

    while (!done_name) {
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

        int done_buf = 0;
        int iResult = 0;
        iResult = recv(ConnectSocket, (char*)&done_buf, sizeof(done_buf), 0);
        if (iResult == 0 || iResult == -1) {
            std::cout << "Lost connection to server!" << std::endl;
            return;
        }

        if (done_buf == 1) {
            done_name = true;
            player_name = name_buf;
        }
    }
    
    /////////////////////////////////////////////////////////////////
    // Recieve info about room

    int clients_buf = 0;
    int iResult = 0;
    iResult = recv(ConnectSocket, (char *)&clients_buf, sizeof(clients_buf), 0);
    if (iResult == 0 || iResult == -1) {
        std::cout << "Lost connection to server!" << std::endl;
        return;
    }
    int clients_count = clients_buf;

    /////////////////////////////////////////////////////////////////

    auto future = std::async(std::launch::async, handle_data);

    if (clients_count == 1) {
        room_setup(ConnectSocket);
        //recieve_setup(ConnectSocket); // recieve setup with player info
        //start_game();
    }
    else if (done_setup == 0) {
        ddd = 1;
        //auto future = std::async(std::launch::async, recieve_setup, std::ref(ConnectSocket));
        if (!wait_for_setup()) {
            disconnect(server_sock);
            return;
        }
        if (done_setup == -1) {
            room_setup(ConnectSocket);
        }
    }

    //auto future = std::async(std::launch::async, handle_data);
    start_game();
}