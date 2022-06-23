#pragma once

#include <WinSock2.h>
#include "role.h"
#include <string>

typedef int role_code;

class Player
{
public:
	std::string name;
	int id;
	role_code role;

	Player() = default;
	Player(std::string name, role_code role);
	Player(std::string name, int id, role_code role);
	~Player() = default;

private:

protected:
	static int next_id;
};

bool operator==(const Player& a, const Player& b);

//class PlayerClient
//{
//public:
//	std::string name;
//	SOCKET socket;
//	Role* role;
//
//	Player() = default;
//	Player(std::string name, SOCKET socket, Role* role);
//	~Player();
//
//private:
//
//};