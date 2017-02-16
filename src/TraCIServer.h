﻿#pragma once

#ifdef _DEBUG
#define DEBUG true
#else
#define DEBUG false
#endif


#include "shawn/socket.h"
#include "shawn/storage.h"
#include "TraCIAPI/Simulation.h"

#define CMDARG_PORT "--traci_port="

//Command definitions:
//Simulation control:
#define CMD_GETVERSION 0x00
#define CMD_SIMSTEP 0x02
#define CMD_SHUTDOWN 0x7f
//------------------
//Value retrieval:
#define CMD_GETSIMVAR 

//Status response defintions:
#define STATUS_OK 0x00
#define STATUS_ERROR 0xFF
#define STATUS_NIMPL 0x01

//Server definitions
#define API_VERSION 10
#define PLUGIN_VERSION "0.11alpha"
#define DEFAULT_PORT 5000

class TraCIServer
{
public:

	TraCIServer();
	~TraCIServer();
	void run();
	void close();
	static void p_printf(std::string text);

private:

	tcpip::Socket* ssocket;
	tcpip::Storage* outgoing;

	traci_api::Simulation* simulation;

	int port;
	bool running;

	void waitForCommands();
	void cmdSimStep(int target_time) const;
	void parseCommand(tcpip::Storage& storage);
	void writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description) const;
	void writeVersion() const;
	void sendResponse() const;

	//commands
	void cmdShutDown();
};
