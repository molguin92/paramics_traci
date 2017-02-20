#pragma once

#include "socket.h"
#include "storage.h"
#include "Simulation.h"

namespace traci_api
{
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
		void cmdGetSimVar(uint8_t simvar) const;

		void parseCommand(tcpip::Storage& storage);
		void writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description) const;
		void writeVersion() const;
		void sendResponse() const;

		//commands
		void cmdShutDown();
	};

}