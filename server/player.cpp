#include <string>
#include <WinSock2.h>
#include "Role.h"

class Player
{
public:
	std::string name;
	SOCKET socket;
	Role role;

	Player(std::string name, SOCKET socket, Role role);
	~Player();

private:
	
};

Player::Player(std::string name, SOCKET socket, Role role)
{
	this->name = name;
	this->socket = socket;
	this->role = role;
}

Player::~Player()
{
	delete this;
}