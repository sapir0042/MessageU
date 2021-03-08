#include <rsa.h>
#include <modes.h>
#include <aes.h>
#include <filters.h>
#include <immintrin.h>
#include <base64.h>
#include <cassert>
#include "Crypto.h"
#include <fstream>
#include <osrng.h>
#include <files.h>
#include <random>

void Crypto::addUser(std::string name)
{
    users[name] = new Keys();
}

// AES public

void Crypto::addSymmetryKey(std::string name, uint8_t* SymmetryKey)
{
    
    //assert(users.find(name) != users.end());
    memcpy(users[name]->symmetry_key, SymmetryKey, SIZE_SYMMETRY_KEY);
    users[name]->has_symmetry_key = true;
}

bool Crypto::hasSymmetryKey(std::string name)
{
    return users[name]->has_symmetry_key;
}

std::string Crypto::encodeSymmetryKey(std::string data, std::string name)
{
    uint8_t* key = users[name]->symmetry_key;
    //std::string data_string(data, size_data);
    return encryptSymmetryKey(key, data);
}

std::string Crypto::decodeSymmetryKey(char* data, size_t size_data, std::string name)
{
    
    uint8_t* key = users[name]->symmetry_key;
    std::string data_string(data, size_data);
    return decryptSymmetryKey(key, data_string);
}

// AES private

uint8_t* Crypto::createSymmetryKey()
{
    CryptoPP::byte key[CryptoPP::AES::DEFAULT_KEYLENGTH], iv[CryptoPP::AES::BLOCKSIZE];

    memset(key, 0x00, CryptoPP::AES::DEFAULT_KEYLENGTH);
    memset(iv, 0x00, CryptoPP::AES::BLOCKSIZE);

    generate_key(reinterpret_cast<char*>(key), CryptoPP::AES::DEFAULT_KEYLENGTH);
    return reinterpret_cast<uint8_t*>(key);
}

char* Crypto::generate_key(char* buff, size_t size)
{
    for (size_t i = 0; i < size; i += 4)
        _rdrand32_step(reinterpret_cast<unsigned int*>(&buff[i]));
    return buff;
}

std::string Crypto::encryptSymmetryKey(uint8_t* key, std::string plaintext)
{
    std::string ciphertext;
    uint8_t iv[SIZE_SYMMETRY_KEY];
    memset(iv, 0x00, SIZE_SYMMETRY_KEY);

    CryptoPP::AES::Encryption aesEncryption(key, SIZE_SYMMETRY_KEY);
    CryptoPP::CBC_Mode_ExternalCipher::Encryption cbcEncryption(aesEncryption, iv);

    CryptoPP::StreamTransformationFilter stfEncryptor(cbcEncryption, new CryptoPP::StringSink(ciphertext));
    stfEncryptor.Put(reinterpret_cast<const unsigned char*>(plaintext.c_str()), plaintext.length());
    stfEncryptor.MessageEnd();

    return ciphertext;
}

std::string Crypto::decryptSymmetryKey(uint8_t* key, std::string ciphertext)
{
    std::string decryptedtext;
    uint8_t iv[SIZE_SYMMETRY_KEY];
    memset(iv, 0x00, SIZE_SYMMETRY_KEY);
    CryptoPP::AES::Decryption aesDecryption(key, SIZE_SYMMETRY_KEY);
    CryptoPP::CBC_Mode_ExternalCipher::Decryption cbcDecryption(aesDecryption, iv);

    CryptoPP::StreamTransformationFilter stfDecryptor(cbcDecryption, new CryptoPP::StringSink(decryptedtext));
    stfDecryptor.Put(reinterpret_cast<const unsigned char*>(ciphertext.c_str()), ciphertext.size());
    stfDecryptor.MessageEnd();

    return decryptedtext;
}

std::string Crypto::getPrivateKey()
{
    return privateKey;
}

void Crypto::setPrivateKey(std::string private_Key)
{
    privateKey = private_Key;
}


// RSA public 

void Crypto::createRsaKey(uint8_t* public_key)
{     

    // private key generation
    std::string private_key_string;
    CryptoPP::AutoSeededRandomPool rng;
    CryptoPP::InvertibleRSAFunction privkey;
    privkey.Initialize(rng, 1024);
    CryptoPP::Base64Encoder privkeysink(new CryptoPP::StringSink(private_key_string));
    privkey.DEREncode(privkeysink);
    privkeysink.MessageEnd();
    privateKey = private_key_string;
   
    // public key generation
    CryptoPP::RSAFunction pubkey(privkey);
    std::string public_key_string;
    CryptoPP::Base64Encoder pubkeysink(new CryptoPP::StringSink(public_key_string));
    pubkey.DEREncode(pubkeysink);
    pubkeysink.MessageEnd();

    // Save the key 
    CryptoPP::ByteQueue bytes;
    CryptoPP::StringSource pkey(public_key_string, true, new CryptoPP::Base64Decoder);
    pkey.TransferTo(bytes);
    bytes.MessageEnd();
    CryptoPP::RSA::PublicKey pubKey;
    pubKey.Load(bytes);

    CryptoPP::ArraySink as(public_key, SIZE_PUBLIC_KEY);
    pubKey.Save(as);
  
}

void Crypto::addPublicKey(std::string name, char* publicKey)
{
    memcpy(users[name]->public_key, publicKey, SIZE_PUBLIC_KEY);
    users[name]->has_public_key = true;
}

bool Crypto::hasPublicKey(std::string name)
{
    return users[name]->has_public_key;
}

std::string Crypto::encodePublicKey(std::string name)
{
    uint8_t* SymmetryKey = createSymmetryKey();
    addSymmetryKey(name, SymmetryKey);
    std::string sym_string((char*)users[name]->symmetry_key, SIZE_SYMMETRY_KEY);
    return encryptPublicKey((uint8_t*)users[name]->public_key, sym_string);
}

void Crypto::decodePublicKey(char* data, size_t size_data, std::string name)
{
    std::string data_string(data, size_data);
    std::string sym_string =  decryptPublicKey(privateKey, data_string);
    uint8_t key[SIZE_SYMMETRY_KEY];
    memcpy((uint8_t*)key, sym_string.c_str(), sym_string.size());
    addSymmetryKey(name, key);

}

// RSA private

std::string Crypto::encryptPublicKey(uint8_t* key, std::string plaintext)
{
    CryptoPP::AutoSeededRandomPool rng;

    // from uint8_t buffer to public key
    CryptoPP::ArraySource as(key, SIZE_PUBLIC_KEY, true);
    CryptoPP::RSA::PublicKey pubKey;
    pubKey.Load(as);

    // encrypt plain text
    std::string ciphertext;
    CryptoPP::RSAES_OAEP_SHA_Encryptor e(pubKey);
    CryptoPP::StringSource ss(plaintext, true, new CryptoPP::PK_EncryptorFilter(rng, e, new CryptoPP::StringSink(ciphertext)));

    return ciphertext;
}

std::string Crypto::decryptPublicKey(std::string key, std::string ciphertext)
{
    CryptoPP::AutoSeededRandomPool rng;

    // read private key from string
    CryptoPP::ByteQueue bytes;
    CryptoPP::StringSource keystring(key, true, new CryptoPP::Base64Decoder);
    keystring.TransferTo(bytes);
    bytes.MessageEnd();
    CryptoPP::RSA::PrivateKey privateKey;
    privateKey.Load(bytes);

    // decrypt ciphertext
    std::string decrypted;
    CryptoPP::RSAES_OAEP_SHA_Decryptor d(privateKey);
    CryptoPP::StringSource ss(ciphertext, true, new CryptoPP::PK_DecryptorFilter(rng, d, new CryptoPP::StringSink(decrypted)));

    return decrypted;
}