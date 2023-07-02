#pragma once

#include <string>

#define R_NONE 0
#define R_VILLAGER 1
#define R_DOCTOR 2
#define R_COP 3
#define R_ESCORT 4
#define R_ARMSDEALER 5
#define R_GODFATHER 6
#define R_MAFIOSO 7
#define R_JESTER 8

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