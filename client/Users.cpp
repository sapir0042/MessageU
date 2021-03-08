#include "Users.h"
#include <cassert>
#include <iostream>

void Users::addUser(uint8_t* id, std::string name)
{			 
	Id *i = new Id();
	memcpy(i, id, 16);
	usersByName[name] = i;

	usersByID[i] = name;
}

std::string Users::getNameByID(uint8_t* id)
{
	Id* i = new Id();
	memcpy(i, id, 16);
	return usersByID[i];
}

const uint8_t* Users::getIDByName(std::string name)
{
	return usersByName[name]->id;
}

bool Users::userExists(std::string name)
{
	return (usersByName.find(name) == usersByName.end()) ? false : true;
}

Users::~Users()
{
	for (const auto& id_name : usersByID) {
		delete[] id_name.first;
	}
}

bool Users::emptyUsers() const
{
	return usersByName.empty();
}
