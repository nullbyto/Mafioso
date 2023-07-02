#include "player.h"
#include "role.h"
#include "room.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>
#include <list>

Room::Room(Roles roles, Settings settings, std::list<Player> players)
{
	this->roles = roles;
	this->settings = settings;
	this->players = players;
}