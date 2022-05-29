#pragma once

#include <WinSock2.h>
#include "role.h"
#include <string>

class Player
{
public:
	std::string name;
	SOCKET socket;
	Role role;

	Player() = default;
	Player(std::string name, SOCKET socket, Role role);
	~Player();

private:
	
};