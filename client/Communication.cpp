#include "Communication.h"
										
void Communication::openSocket()
{
	sock = new tcp::socket(io_context);
	tcp::resolver resolver(io_context);
	boost::asio::connect(*sock, resolver.resolve(address, port));
} 

void Communication::getServerInfo()
{
	std::fstream serverinfo("server.info");
	if (serverinfo) {
		std::string p;
		std::getline(serverinfo, address, ':');
		std::getline(serverinfo, port);
		serverinfo.close();
	}
	else {
		std::cerr << "Error: Unable to open server.info file.\n";
		exit(-1);
	}
}

Communication::Communication()
{	
	getServerInfo();
}

void Communication::send(const char* buffer, size_t sizeBuffer) const
{
	sock->send(boost::asio::buffer(buffer, sizeBuffer));
}

void Communication::receive(char* buffer, size_t sizeBuffer)
{
	sock->receive(boost::asio::buffer(buffer, sizeBuffer));
}

void Communication::closeSocket()
{
	sock->close();
}
