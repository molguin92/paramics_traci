#pragma once
#include <cstdint>
#include "storage.h"

namespace traci_api
{


	class Simulation
	{
	public:

		// Command constants:
		static const uint8_t GET_SIMTIME = 0x70;

		Simulation();
		~Simulation();

		int runSimulation(uint32_t target_time, tcpip::Storage& result_store);
		bool getVariable(uint8_t varID, tcpip::Storage& result_store);
	private:
		float getCurrentTimeSeconds();
		int getCurrentTimeMilliseconds();
		int stepcnt;
	};
}

