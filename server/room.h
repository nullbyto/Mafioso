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

	std::vector<Role> roles;
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