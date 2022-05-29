#include "player.h"
#include "role.h"
#include "room.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>


Room::Room(int count, std::string name, Player leader, std::vector<Player> players, Settings settings)
{
	this->count = count;
	this->name = name;
	this->leader = leader;
	this->players = players;
	this->settings = settings;
}
Room::~Room() {

}