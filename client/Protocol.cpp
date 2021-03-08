#include "Protocol.h"
#include "MeInfo.h"
#include <boost/algorithm/hex.hpp>

Protocol::Protocol()
{
	comm = new Communication();
	users = new Users();
	crypto = new Crypto();
	if (MeInfo::existsFile())
	{
		std::string idFromFile, privateKeyFromFile;
		MeInfo::readFromTheFile(&idFromFile, &privateKeyFromFile);
		memcpy(requestHeader.client_id, idFromFile.c_str(), ID_SIZE);
		crypto->setPrivateKey(privateKeyFromFile);
	}
}

void Protocol::run()
{
	bool exit = true;
	while (exit != false)
	{
		comm->openSocket();
		printMenu();
		int requestClient = getsInput();
		switch (requestClient)
		{
		case 0:	exit = false; 
			break;
		case 1: requestRegistration(); 
			break;
		case 2: requestUsersList();
			break;
		case 3:	requestPublicKey();
			break;
		case 4:	retrieveWaitingMessages();
			break;
		case 5:	sendMessage();
			break;
		case 51:requestSymmetricKey();
			break;
		case 52:sendSymmetricKey();
			break;
		default:std::cout << "Not a Valid Choice. \n";
				std::cout << "Choose again.\n";
			break;
		}
		comm->closeSocket();
	}
}

void Protocol::requestRegistration()
{
	if (MeInfo::existsFile()) {
		std::cout << "error: you are already registration" << std::endl;
		return;
	}
		
	// fill request payload
	RequestRegistrationPayload requestPayload;
	std::string name = getsNameClient();
	memcpy(requestPayload.name, name.c_str(), MAX_SIZE_NAME); // fill name in request payload
	crypto->createRsaKey(requestPayload.public_key); // fill public key in request payload

	// fill request header
	fillRequestHeader(REQUEST_REGISTRATION, PAYLOAD_SIZE_REGISTER);

	// send request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));
	comm->send(reinterpret_cast<char*>(&requestPayload), sizeof(RequestRegistrationPayload));

	// receive response
	ResponseHeader responseHeader;
	ResponseSesuccessfulRegistrationPayload payloadResponse;
	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));
	comm->receive(reinterpret_cast<char*>(&payloadResponse), responseHeader.payload_size);
		
	if (!validResponseHeader(responseHeader.version, responseHeader.code, RESPONSE_REGISTRATION)){
		std::cout << "error: the name already exist in the server" << std::endl;
		return;
	}
	std::string id((char*)payloadResponse.client_id, ID_SIZE);
	MeInfo::createFileAndWrite((char*)requestPayload.name, id, crypto->getPrivateKey()); // write in the file: name, id and private key
	memcpy(requestHeader.client_id, id.c_str(), ID_SIZE); // fill id in request header
}

void Protocol::requestUsersList()
{
	// fill request header
	fillRequestHeader(REQUEST_USERS_LIST, ZERO);

	// send the request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));

	// receive header response
	ResponseHeader responseHeader;
	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));

	if (!validResponseHeader(responseHeader.version, responseHeader.code, RESPONSE_USERS_LIST)) {
		errorResponse(responseHeader.payload_size);
		return;
	}

	std::cout << "Users list:"<< std::endl;
	uint8_t userCounter = (responseHeader.payload_size) / (sizeof(ResponseUsersListPayload));

	// receive payload response
	ResponseUsersListPayload payloadResponse;
	for (; userCounter; userCounter--){
		comm->receive(reinterpret_cast<char*>(&payloadResponse), sizeof(ResponseUsersListPayload));
		std::string id((char*)payloadResponse.client_id, ID_SIZE);
		std::string name((char*)payloadResponse.client_name);
		if (!users->userExists(name)) {
			users->addUser(payloadResponse.client_id, name);
			crypto->addUser(name);
		}
		std::cout << name << std::endl;
	}
}

void Protocol::requestPublicKey()
{
	std::string name = getsNameClient();
	if (!users->userExists(name)) {
		std::cout << "The client not exist in your clients list" << std::endl;
		return;
	}

	if (crypto->hasPublicKey(name)) {
		std::cout << "You already have the public key of this client" << std::endl;
		return;
	}
	// fill request payload
	RequestPublicKeyPayload requestPayload;
	memcpy(requestPayload.client_id, users->getIDByName(name), ID_SIZE); // fill id in request payload
	
	// fill request header
	fillRequestHeader(REQUEST_PUBLIC_KEY, ID_SIZE);

	// send the request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));
	comm->send(reinterpret_cast<char*>(&requestPayload), sizeof(RequestPublicKeyPayload));

	// receive response
	ResponseHeader responseHeader{};
	ResponsePublicKeyPayload payloadResponse;
	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));
	comm->receive(reinterpret_cast<char*>(&payloadResponse), responseHeader.payload_size);

	if (!validResponseHeader(responseHeader.version, responseHeader.code, RESPONSE_PUBLIC_KEY)) {
		std::cout << "server responded with an error" << std::endl;
		return;
	}
	crypto->addPublicKey(name, (char*)payloadResponse.public_key);
	std::cout << "The public key was successfully received." << std::endl;
}

void Protocol::retrieveWaitingMessages()
{
	if (users->emptyUsers()) {
		std::cout << "You dont have a clients list" << std::endl;
		std::cout << "Please request clients list before request waiting messages" << std::endl;
		return;
	}
	// fill request header
	fillRequestHeader(REQUEST_WAITING_MESSAGES, ZERO);

	// send the request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));

	// receive response
	ResponseHeader responseHeader;
	ResponseWaitingMessagePayload payloadResponse;

	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));
	uint32_t counterBytes = 0;
	
	if (!validResponseHeader(responseHeader.version, responseHeader.code,RESPONSE_WAITING_MESSAGES)) {
		errorResponse(responseHeader.payload_size);
		return;
	}

	if (responseHeader.payload_size == 0) {
		std::cout << "no waiting messages." << std::endl;
		return;
	}

	while(counterBytes < responseHeader.payload_size){
		bool print = true;
		// receive payload
		comm->receive(reinterpret_cast<char*>(&payloadResponse), sizeof(ResponseWaitingMessagePayload));

		// receive content payload
		char* buffer = new char[payloadResponse.message_size];
		comm->receive(reinterpret_cast<char*>(buffer), payloadResponse.message_size);

		std::string name(users->getNameByID(payloadResponse.client_id));
		std::string content = "";
		switch (payloadResponse.message_type)
		{
		case 1: content = "Request for symmetric key";
			break;
		case 2: crypto->decodePublicKey(buffer, payloadResponse.message_size, name);
				content = "received symmetric key";
			break;
		case 3:	content = crypto->decodeSymmetryKey(buffer, payloadResponse.message_size, name);
			break;
		}
		if(print)
			printMessages(name, content);
		counterBytes += payloadResponse.message_size + sizeof(ResponseWaitingMessagePayload);
		delete[] buffer;
	}
}

void Protocol::sendMessage()
{
	std::string name = getsNameClient();
	if (!users->userExists(name)) {
		std::cout << "The client not exist in your clients list" << std::endl;
		return;
	}	
	if (!crypto->hasSymmetryKey(name)) {
		std::cout << "You dont have a symmetric key for this client" << std::endl;
		return;
	}

	// fill request payload
	RequestSendMessagePayload requestPayload;
	std::string msg = getsMessage(); // gets message from the client
	std::string encode_msg;

	requestPayload.message_type = SEND_MESSAGE;	// fill message type  in request payload
	encode_msg = crypto->encodeSymmetryKey(msg, name); // encode the message

	char* buffer = new char[encode_msg.size()];

	memcpy(buffer, encode_msg.c_str(), encode_msg.size()); // fill the encode message in request payload
	memcpy(requestPayload.client_id, users->getIDByName(name), ID_SIZE); // fill id in request payload
	requestPayload.content_size = encode_msg.size(); // fill content size in request payload
	
	// fill request header
	fillRequestHeader(REQUEST_SEND_MESSAGE, encode_msg.size() + sizeof(requestPayload));
	
	// send the request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));
	comm->send(reinterpret_cast<char*>(&requestPayload), sizeof(RequestSendMessagePayload));
	comm->send(reinterpret_cast<char*>(buffer), encode_msg.size());
	delete[] buffer;

	// receive response
	ResponseHeader responseHeader;
	ResponseMessageSentSuccessfullyPayload payloadResponse;
	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));
	comm->receive(reinterpret_cast<char*>(&payloadResponse), responseHeader.payload_size);

	if (validResponseHeader(responseHeader.version, responseHeader.code, RESPONSE_MESSAGE_SENT))
		std::cout << "The message sent successfully"  << std::endl;
	else
		std::cout << "server responded with an error" << std::endl;
}

void Protocol::requestSymmetricKey()
{
	std::string name = getsNameClient();
	if (!users->userExists(name)) {
		std::cout << "The client not exist in your clients list" << std::endl;
		return;
	}
	if (!crypto->hasPublicKey(name)) {
		std::cout << "You must first request the client's public key" << std::endl;
		return;
	}

	// fill  request payload
	RequestSendMessagePayload requestPayload;
	memcpy(requestPayload.client_id, users->getIDByName(name), ID_SIZE); // fill id in request payload
	requestPayload.message_type = REQUEST_SYM_KEY; // fill message type  in request payload
	requestPayload.content_size = ZERO; // fill content size in request payload

	// fill request header
	fillRequestHeader(REQUEST_SEND_MESSAGE, sizeof(RequestSendMessagePayload));

	// send the request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));
	comm->send(reinterpret_cast<char*>(&requestPayload), sizeof(RequestSendMessagePayload));

	// receive response
	ResponseHeader responseHeader;
	ResponseMessageSentSuccessfullyPayload payloadResponse;
	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));
	comm->receive(reinterpret_cast<char*>(&payloadResponse), responseHeader.payload_size);

	if (validResponseHeader(responseHeader.version, responseHeader.code, RESPONSE_MESSAGE_SENT))
		std::cout << "The message sent successfully" << std::endl;
	else
		std::cout << "server responded with an error" << std::endl;
}

void Protocol::sendSymmetricKey()
{
	std::string name = getsNameClient();
	if (!users->userExists(name)) {
		std::cout << "The client not exist in your clients list" << std::endl;
		return;
	}
	if (crypto->hasSymmetryKey(name)) {
		std::cout << "You already have a symmetric key for this client" << std::endl;
		return;
	}
	if (!crypto->hasPublicKey(name)) {
		std::cout << "You must first request the client's public key" << std::endl;
		return;
	}
	// fill request payload
	RequestSendMessagePayload requestPayload;	

	std::string encode_msg = crypto->encodePublicKey(name); // encode the message
	char* buffer = new char[encode_msg.size()];

	memcpy(buffer, encode_msg.c_str(), encode_msg.size()); // fill the encode message in request payload
	memcpy(requestPayload.client_id, users->getIDByName(name), ID_SIZE); // fill id in request payload
	requestPayload.message_type = RETRIEVE_SYM_KEY;	// fill message type  in request payload
	requestPayload.content_size = encode_msg.size(); // fill content size in request payload
	
	// fill request header
	fillRequestHeader(REQUEST_SEND_MESSAGE, sizeof(RequestSendMessagePayload) + encode_msg.size());

	// send the request
	comm->send(reinterpret_cast<char*>(&requestHeader), sizeof(RequestHeader));
	comm->send(reinterpret_cast<char*>(&requestPayload), sizeof(RequestSendMessagePayload));
	comm->send(reinterpret_cast<char*>(buffer), encode_msg.size());
	delete[] buffer;

	// receive response
	ResponseHeader responseHeader;
	ResponseMessageSentSuccessfullyPayload payloadResponse;
	comm->receive(reinterpret_cast<char*>(&responseHeader), sizeof(ResponseHeader));
	comm->receive(reinterpret_cast<char*>(&payloadResponse), responseHeader.payload_size);

	if (validResponseHeader(responseHeader.version, responseHeader.code, RESPONSE_MESSAGE_SENT))
		std::cout << "The message sent successfully" << std::endl;
	else
		std::cout << "server responded with an error" << std::endl;
}

void Protocol::fillRequestHeader(uint8_t code, uint32_t payload_size)
{
	requestHeader.code = code;
	requestHeader.payload_size = payload_size;
}

// return true if valid response header
bool Protocol::validResponseHeader(uint8_t responseVersion, uint16_t responseCode, uint16_t code)
{
	return responseVersion == SRV_VERSION && responseCode == code;
}

void Protocol::errorResponse(uint32_t size) 
{
	char* buffer = new char[size];
	comm->receive(reinterpret_cast<char*>(buffer), size);
	delete[] buffer;
	std::cout << "server responded with an error" << std::endl;
}

void Protocol::printMessages(std::string userName, std::string content)
{
	std::cout << "From: " << userName << std::endl;
	std::cout << "Content: " << std::endl;
	std::cout << content << std::endl;
	std::cout << ".\n." << std::endl;
	std::cout << "----<EOM>----" << std::endl;
	std::cout << "\\n" << std::endl;
}

void Protocol::printMenu() {
	std::cout << "\nMessageU client at your service.\n" << std::endl;
	std::cout << "1) Register" << std::endl;
	std::cout << "2) Request for clients list" << std::endl;
	std::cout << "3) Request for public key" << std::endl;
	std::cout << "4) Request for waiting messagea" << std::endl;
	std::cout << "5) Send a text message" << std::endl;
	std::cout << "51) Send a request for symmetric key" << std::endl;
	std::cout << "52) Send your symmetric key" << std::endl;
	std::cout << "0) Exit client" << std::endl;
	std::cout << "?" << std::endl;
}

// gets name from the client
std::string Protocol::getsNameClient()
{
	std::string name;
	std::cout << "Enter name: ";
	do
	{
		std::getline(std::cin >> std::ws, name);
		if (name.length() < MAX_SIZE_NAME)
			return name;
		std::cout << "You can type up to " << int(MAX_SIZE_NAME) << " characters.\ntry again.\n";
	} while (true);
}

// gets message from the client
std::string Protocol::getsMessage()
{
	std::string message;
	std::cout << "Enter the message: ";
	std::getline(std::cin >> std::ws, message);
	return message;
}

// Returns true if s is a number else false
bool Protocol::isNumber(std::string s)
{
	for (int i = 0; i < s.length(); i++)
		if (isdigit(s[i]) == false)
			return false;

	return true;
}

// get inupt from the client 
int Protocol::getsInput() {
	int num;
	std::string in;
	std::cin >> in;

	if (!isNumber(in))
		num = 9000;	 // invalid number
	else 
	{
		std::stringstream number(in);
		number >> num;
	}
	return num;
}

