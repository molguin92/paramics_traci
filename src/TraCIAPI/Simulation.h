#pragma once
#include "storage.h"
#include "programmer.h"
#include <mutex>

namespace traci_api
{


	class Simulation
	{
	public:

		// Command constants:
		// simulation time
		static const uint8_t GET_SIMTIME = 0x70;

		// departed vehicles
		static const uint8_t GET_RELEASEDVHC_CNT = 0x73;
		static const uint8_t GET_RELEASEDVHC_LST = 0x74;

		Simulation();
		~Simulation();

		int runSimulation(uint32_t target_time, tcpip::Storage& result_store);
		bool getVariable(uint8_t varID, tcpip::Storage& result_store);

		void releaseVehicle(VEHICLE* vehicle);

	private:

		std::mutex *lock = new std::mutex();
		std::vector<VEHICLE*> released_vehicles;
		float getCurrentTimeSeconds();
		int getCurrentTimeMilliseconds();
		int stepcnt;
	};
}

