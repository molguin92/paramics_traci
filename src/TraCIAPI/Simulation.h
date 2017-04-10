#pragma once
#include "storage.h"
#include <mutex>
#include "Utils.h"

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

		// network boundary box
		static const uint8_t GET_NETWORKBNDS = 0x7c;

		//Simulation();
		~Simulation();
		// prevent alternative instantiation
		Simulation(Simulation const&) = delete;
		void operator =(Simulation const&) = delete;

		static Simulation* getInstance();
		static void deleteInstance();

		int runSimulation(uint32_t target_time, tcpip::Storage& result_store);
		bool getVariable(uint8_t varID, tcpip::Storage& result_store);

		float getCurrentTimeSeconds();
		int getCurrentTimeMilliseconds();

		BoundaryBox getNetworkBoundaries();

	private:

		static Simulation* instance;
		Simulation();
		int stepcnt;
	};
}

