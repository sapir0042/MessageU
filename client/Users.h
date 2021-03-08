#ifndef __USERS_H__
#define __USERS_H__
#include <map>
#include <string>

struct Id {
	uint8_t id[16];
};

class idLess {
public:
	bool operator()(const Id* a, const Id* b) const {
		for (int i = 0; i < 16; i++) {
			if (a->id[i] != b->id[i]) {
				return a->id[i] < b->id[i];
			}
		}
		return false;
	}
};

class Users
{
public:
	void addUser(uint8_t *id, std::string name);
	std::string getNameByID(uint8_t* id);
	const uint8_t* getIDByName(std::string name);
	bool userExists(std::string name);
	bool emptyUsers() const;
	~Users();

private:
	std::map<std::string, const Id*> usersByName;
	std::map<const Id*, std::string, idLess> usersByID;
};
#endif /* __USERS_H__ */
