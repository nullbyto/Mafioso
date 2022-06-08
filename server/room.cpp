#include "player.h"
#include "role.h"
#include "room.h"
#include <stdlib.h>
#include <iostream>
#include <string>
#include <vector>


Room::Room(Roles roles, Settings settings)
{
	this->roles = roles;
	this->settings = settings;
}