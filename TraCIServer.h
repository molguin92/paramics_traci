#pragma once

#ifdef _DEBUG
#define DEBUG true
#else
#define DEBUG false
#endif


#include "shawn/socket.h"
#include "shawn/storage.h"

//Command definitions:
#define CMD_GETVERSION 0x00
#define CMD_SIMSTEP 0x02

//Status response defintions:
#define STATUS_OK 0x00
#define STATUS_ERROR 0xFF
#define STATUS_NIMPL 0x01

//Server definitions
#define API_VERSION 10
#define PLUGIN_VERSION "0.1alpha"

class TraCIServer
{
public:

	tcpip::Socket* ssocket;
	tcpip::Storage* outgoing;

	TraCIServer();
	~TraCIServer();
	void run();
	void close();

private:
	void waitForCommands();
	void parseCommand(tcpip::Storage& storage);
	void writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description);
	void writeVersion();
	void sendResponse();
	void p_printf(std::string text);

	//commands
	void cmdSimulationStep();

};
