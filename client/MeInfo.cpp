#include "MeInfo.h"
#include <boost/algorithm/hex.hpp>
#include <boost/asio.hpp>


bool MeInfo::existsFile()
{
    std::ifstream meFile(NAME_FILE);
    return (bool)meFile;
}

void MeInfo::createFileAndWrite(std::string name, std::string id, std::string privateKey)
{
    std::ofstream myFile(NAME_FILE);
    if (myFile)
    {
        myFile << name << std::endl;
        myFile << stringToHex(id) << std::endl;
        myFile << privateKey; // Make sure it is at base 64
        myFile.close();
    }
    else {
        std::cerr << "Error: Unable to open " << NAME_FILE <<" file." << std::endl;
    }
}

void MeInfo::readFromTheFile(std::string* id, std::string* privateKey)
{
    std::ifstream meFile(NAME_FILE);
    if (meFile) {
        std::string name, id_string, key, buffer;
        std::getline(meFile, name);
        std::getline(meFile, id_string);
        while (meFile >> buffer)
            key += buffer;
        meFile.close();
        *id = hexToString(id_string);
        *privateKey = key;
    }
    else 
        std::cerr << "Error: Unable to open " << NAME_FILE << " file." << std::endl;
    
}

std::string MeInfo::stringToHex(const std::string str)
{
    std::string hexStr;
    boost::algorithm::hex_lower(str.begin(), str.end(), std::back_inserter(hexStr));
    return hexStr;
}

std::string MeInfo::hexToString(const std::string str)
{
    std::string strHex;
    boost::algorithm::unhex(str.begin(), str.end(), std::back_inserter(strHex));
    return strHex;
}
