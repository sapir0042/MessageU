#ifndef __PROTOCOL_H__
#define __PROTOCOL_H__
#include "Users.h"
#include "Communication.h"
#include "Crypto.h"
#include "RequestStruct.h"
#include "ResponseStruct.h"
#include <cstdint>

constexpr uint8_t SRV_VERSION = 2;
constexpr uint8_t ZERO = 0;
constexpr uint32_t PAYLOAD_SIZE_REGISTER = 415;
constexpr uint8_t MAX_SIZE_NAME = 255;
constexpr uint8_t ID_SIZE = 16;
class Protocol
{
public:
	Protocol();
	void run(); 

private:
	Users* users;
	Communication* comm;
	Crypto* crypto;
	RequestHeader requestHeader;

	void fillRequestHeader(uint8_t code, uint32_t payload_size);
	bool validResponseHeader(uint8_t responseVersion, uint16_t responseCode, uint16_t code);
	void requestRegistration();
	void requestUsersList();
	void errorResponse(uint32_t size);
	void requestPublicKey();
	void retrieveWaitingMessages();
	void sendMessage();
	void sendSymmetricKey();
	void requestSymmetricKey();
	void printMessages(std::string userName, std::string content);
	void printMenu();
	std::string getsNameClient();
	std::string getsMessage();
	bool isNumber(std::string s);
	int getsInput();
};
#endif /* __PROTOCOL_H__ */