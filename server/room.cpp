#include "player.h"
#include "role.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>

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


	Room(int count, std::string name, Player leader, std::vector<Player> players, Settings settings)
	{
		this->count = count;
		this->name = name;
		this->leader = leader;
		this->players = players;
		this->settings = settings;
	}
	~Room()
	{
		delete this;
	}

private:
	
};


