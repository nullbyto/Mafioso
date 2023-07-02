#include <string>
#include <WinSock2.h>
#include "Role.h"
#include "player.h"

int Player::next_id = 0;

//Player::Player() {
//	this->name = "";
//	this->role = {};
//	this->id = ++next_id;
//}

Player::Player(std::string name, role_code role)
{
	this->name = name;
	this->role = role;
	this->id = ++next_id;
}

Player::Player(std::string name, int id, role_code role){
	this->name = name;
	this->role = role;
	this->id = id;
}

bool operator==(const Player& a, const Player& b) {
	return a.name == b.name;
};