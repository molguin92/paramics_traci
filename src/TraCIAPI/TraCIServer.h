#pragma once

#include "socket.h"
#include "storage.h"
#include "Simulation.h"

namespace traci_api
{
	class TraCIServer
	{
	public:

		TraCIServer(int port);
		~TraCIServer();
		void run();
		void close();
		static void p_printf(std::string text);

	private:

		tcpip::Socket ssocket;
		tcpip::Storage outgoing;
		bool running;
		int port;

		void waitForCommands();
		void cmdSimStep(int target_time);
		void cmdGetSimVar(uint8_t simvar);
		void cmdGetVhcVar(tcpip::Storage& input);
		void cmdSetVhcState(tcpip::Storage& state);

		void parseCommand(tcpip::Storage& storage);
		void writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description);
		void writeVersion();
		void sendResponse();

		void writeToOutputWithSize(tcpip::Storage& storage);

		//commands
		void cmdShutDown();
	};

}