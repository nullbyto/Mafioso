#pragma once

#include <string>
#include <vector>
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
    Roles roles;
	Settings settings;

    Room() = default;
	Room(Roles roles, Settings settings);
	~Room() = default;

private:

};