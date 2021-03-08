#ifndef __COMMUNICATION_H__
#define __COMMUNICATION_H__
#include <boost/asio.hpp>
#include <fstream>
#include <iostream>

using boost::asio::ip::tcp;
class Communication
{
private:
	boost::asio::io_context io_context;
	tcp::socket *sock;
	tcp::resolver* resolver;
	std::string address;
	std::string port;
	void getServerInfo();

public:
	Communication();
	void openSocket();
	void send(const char* buffer, size_t sizeBuffer) const;
	void receive(char* buffer, size_t sizeBuffer);
	void closeSocket();
};
#endif /* __COMMUNICATION_H__ */
