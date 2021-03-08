#ifndef __CRYPTO_H__
#define __CRYPTO_H__
#include <string>
#include <map>

constexpr size_t SIZE_PUBLIC_KEY = 160;
constexpr size_t SIZE_SYMMETRY_KEY = 16;

struct Keys {
	uint8_t public_key[SIZE_PUBLIC_KEY] = { 0 };
	uint8_t symmetry_key[SIZE_SYMMETRY_KEY] = { 0 };
	bool has_public_key = false;
	bool has_symmetry_key = false;
};

class Crypto
{
public:
	void addUser(std::string name);

	// RSA
	void createRsaKey(uint8_t* public_key);
	void addPublicKey(std::string name, char* publicKey);
	bool hasPublicKey(std::string name);
 	std::string encodePublicKey(std::string name);
	void decodePublicKey(char* data, size_t size_data, std::string name);

	// AES 
 	void addSymmetryKey(std::string name, uint8_t* SymmetryKey);
	bool hasSymmetryKey(std::string name);	 
	std::string getPrivateKey();
	void setPrivateKey(std::string privateKey);
	std::string encodeSymmetryKey(std::string data, std::string name);
	std::string decodeSymmetryKey(char* data, size_t size_data, std::string name);
	
private:
	std::string privateKey;
	std::map<std::string, Keys*> users;

	// RSA
	std::string encryptPublicKey(uint8_t* key, std::string plaintext);
	std::string decryptPublicKey(std::string key, std::string ciphertext);
	
	// AES 
	uint8_t* createSymmetryKey();
	char* generate_key(char* buff, size_t size);
	std::string encryptSymmetryKey(uint8_t* key, std::string plaintext);
	std::string decryptSymmetryKey(uint8_t* key, std::string ciphertext);
};
#endif /* __CRYPTO_H__ */
