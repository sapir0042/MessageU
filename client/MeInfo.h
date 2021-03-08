#ifndef __MEINFO_H__
#define __MEINFO_H__
#include <string>
#include <fstream>
#include <iostream>
const std::string NAME_FILE = "me.info";
class MeInfo
{	
public:
	static bool existsFile();
	static void createFileAndWrite(std::string name, std::string id, std::string privateKey);
	static void readFromTheFile(std::string* id, std::string* privateKey);
	static std::string stringToHex(const std::string str);
	static std::string hexToString(const std::string str);

};
#endif /* __MEINFO_H__ */