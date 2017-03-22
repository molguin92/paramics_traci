#pragma once
#include "storage.h"
#include "programmer.h"
#include <mutex>
#include <unordered_map>
#include "VehicleManager.h"

namespace traci_api
{


	class Simulation
	{
	public:

		// Command constants:
		// simulation time
		static const uint8_t GET_SIMTIME = 0x70;

		// loaded vehicles
		static const uint8_t GET_LOADEDVHC_CNT = 0x71;
		static const uint8_t GET_LOADEDVHC_LST = 0x72;

		// departed vehicles
		static const uint8_t GET_DEPARTEDVHC_CNT = 0x73;
		static const uint8_t GET_DEPARTEDVHC_LST = 0x74;

		// arrived vehicles
		static const uint8_t GET_ARRIVEDVHC_CNT = 0x79;
		static const uint8_t GET_ARRIVEDVHC_LST = 0x7a;

		// simulation timestep size
		static const uint8_t GET_TIMESTEPSZ = 0x7b;

		// set vehicle speed
		static const uint8_t SET_VHCSPEED = 0x40;

		VehicleManager vhcman;

		Simulation();
		~Simulation();

		int runSimulation(uint32_t target_time, tcpip::Storage& result_store);
		bool getVariable(uint8_t varID, tcpip::Storage& result_store);
		void setVhcState(tcpip::Storage& state);

	private:

		float getCurrentTimeSeconds();
		int getCurrentTimeMilliseconds();
		int stepcnt;
	};
}

