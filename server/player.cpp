#include <string>
#include <WinSock2.h>
#include "Role.h"
#include "player.h"


Player::Player(std::string name, SOCKET socket, Role role)
{
	this->name = name;
	this->socket = socket;
	this->role = role;
}

Player::~Player() {

}