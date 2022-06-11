#pragma once

#include <string>

class Role
{
public:
	std::string action_name;
	bool is_saved;
	bool is_escorted;
	virtual void action() = 0;
private:

};

class Village : public Role {
public:

private:

};

class Mafia : public Role {
public:

private:

};

class Independent : public Role {
public:

private:

};

// -- Village --------------------------
class Villager : public Village {

};

class Doctor : public Village {

};

class Cop : public Village {

};

class Escort : public Village {

};

class Armsdealer : public Village {

};


// -- Mafia --------------------------
class Godfather : public Mafia {

};

class Mafioso : public Mafia {

};


// -- Independent --------------------------
class Jester : public Independent {

};