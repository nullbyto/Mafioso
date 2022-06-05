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
    bool villager;
    bool doctor;
    bool cop;
    bool escort;
    bool armsdealer;
    // -- Mafia
    bool godfather;
    bool mafioso;
    // -- Independent
    bool jester;
};

class Room
{
public:
	int count;
	std::string name;
	Player leader;
	std::vector<Player> players;
	Settings settings;

	Room(int count, std::string name, Player leader, std::vector<Player> players, Settings settings);
	~Room();

private:

};