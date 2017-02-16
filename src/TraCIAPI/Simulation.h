#pragma once
#include <cstdint>
#include "../shawn/storage.h"

namespace traci_api
{
	class Simulation
	{
	public:
		Simulation();
		~Simulation();

		float getCurrentTimeSeconds();
		long getCurrentTimeMilliseconds();

		int runSimulation(uint32_t target_time, tcpip::Storage& result_store);
	private:
		int stepcnt;
	};
}

