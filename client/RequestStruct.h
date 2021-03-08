#ifndef __REQUESTSTRUCT_H__
#define __REQUESTSTRUCT_H__

constexpr uint8_t CLIENT_VERSION = 1;

// request codes
enum request_code : uint8_t {
	REQUEST_REGISTRATION = 100, 
	REQUEST_USERS_LIST = 101, 
	REQUEST_PUBLIC_KEY = 102, 
	REQUEST_SEND_MESSAGE = 103, 
	REQUEST_WAITING_MESSAGES = 104
};

// message_type
enum message_type : uint8_t {
	REQUEST_SYM_KEY = 1,
	RETRIEVE_SYM_KEY = 2,
	SEND_MESSAGE = 3,
	SEND_FILE = 4
};

// Basic request header struct
#pragma pack(push, 1)
struct RequestHeader
{
	uint8_t client_id[16] = { 0 };
	uint8_t version = CLIENT_VERSION;
	uint8_t code;
	uint32_t payload_size;
};
#pragma pack(pop)

// request payload struct code 100
#pragma pack(push, 1)
struct RequestRegistrationPayload
{
	uint8_t name[255] = { 0 };
	uint8_t public_key[160] = { 0 };
};
#pragma pack(pop)

// request payload struct code 102
#pragma pack(push, 1)
struct RequestPublicKeyPayload
{
	uint8_t client_id[16] = { 0 };
};
#pragma pack(pop)

// request payload struct code 103.
#pragma pack(push, 1)
struct RequestSendMessagePayload
{
	uint8_t client_id[16] = { 0 };
	uint8_t message_type;
	uint32_t content_size;
};
#endif /* __REQUESTSTRUCT_H__ */