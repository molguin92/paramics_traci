// test_sockets.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "Poco/Net/SocketAddress.h"
#include "Poco/Net/StreamSocket.h"
#include "Poco/Net/SocketStream.h"
#include "Poco/StreamCopier.h"
#include <iostream>


int main(int argc, char** argv)
{
	Poco::Net::SocketAddress sa("google.com", 80);
	Poco::Net::StreamSocket socket(sa);
	Poco::Net::SocketStream str(socket);
	str << "GET / HTTP/1.1\r\n"
		"Host: google.com\r\n"
		"\r\n";
	str.flush();
	Poco::StreamCopier::copyStream(str, std::cout);
	return 0;
}
