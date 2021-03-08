
#include "protocol.h"

int main(int argc, char **argv) {
	try
	{	
		Protocol protocol = Protocol();
		protocol.run();
		return 0;
	}
	catch (std::exception& e)
	{
		std::cerr << "Exception: " << e.what() << "\n";
	}
}  
