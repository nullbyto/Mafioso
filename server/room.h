#pragma once

#include <string>
#include <vector>
#include <list>
#include "player.h"

struct Settings {
	int day_length;
	int night_length;
	bool last_will;
	bool no_reveal;
	bool day_start;
};

struct Roles {
    // -- Village
    int villager;
    int doctor;
    int cop;
    int escort;
    int armsdealer;
    // -- Mafia
    int godfather;
    int mafioso;
    // -- Independent
    int jester;
};

class Room
{
public:
    Roles roles = {};
	Settings settings = {};
    std::list<Player> players;

    Room() = default;
	Room(Roles roles, Settings settings, std::list<Player> players);
	~Room() = default;

private:

};