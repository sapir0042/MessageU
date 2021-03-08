#ifndef __RESPONSESTRUCT_H__
#define __RESPONSESTRUCT_H__

// response codes.
enum response_code : uint16_t {
	RESPONSE_REGISTRATION = 1000, 
	RESPONSE_USERS_LIST = 1001, 
	RESPONSE_PUBLIC_KEY = 1002,
	RESPONSE_MESSAGE_SENT = 1003,
	RESPONSE_WAITING_MESSAGES = 1004
};

// Basic response header struct
#pragma pack(push, 1)
struct ResponseHeader
{
	uint8_t version;
	uint16_t code;
	uint32_t payload_size;
};
#pragma pack(pop)

// response payload struct code 1000
#pragma pack(push, 1)
struct ResponseSesuccessfulRegistrationPayload
{
	uint8_t client_id[16] = { 0 };
};
#pragma pack(pop) 

// response payload struct code 1001
#pragma pack(push, 1)
struct ResponseUsersListPayload
{
	uint8_t client_id[16] = { 0 };
	uint8_t client_name[255] = { 0 };
};
#pragma pack(pop)

// response payload struct code 1002.
#pragma pack(push, 1)
struct ResponsePublicKeyPayload
{
	uint8_t client_id[16] = { 0 };
	uint8_t public_key[160] = { 0 };
};
#pragma pack(pop)

// response payload struct code 1003
#pragma pack(push, 1)
struct ResponseMessageSentSuccessfullyPayload
{
	uint8_t client_id[16] = { 0 };
	uint32_t message_id;
};
#pragma pack(pop)

// response payload struct code 1004
#pragma pack(push, 1)
struct ResponseWaitingMessagePayload
{
	uint8_t client_id[16] = { 0 };
	uint32_t message_id;
	uint8_t message_type;
	uint32_t message_size;
};
#pragma pack(pop)
#endif /* __RESPONSESTRUCT_H__ */