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

		// loaded vehicles
		static const uint8_t GET_LOADEDVHC_CNT = 0x71;
		static const uint8_t GET_LOADEDVHC_LST = 0x72;

		// departed vehicles
		static const uint8_t GET_DEPARTEDVHC_CNT = 0x73;
		static const uint8_t GET_DEPARTEDVHC_LST = 0x74;

		// arrived vehicles
		static const uint8_t GET_ARRIVEDVHC_CNT = 0x79;
		static const uint8_t GET_ARRIVEDVHC_LST = 0x7a;

		Simulation();
		~Simulation();

		int runSimulation(uint32_t target_time, tcpip::Storage& result_store);
		bool getVariable(uint8_t varID, tcpip::Storage& result_store);

		void vehicleDepart(VEHICLE* vehicle);
		void vehicleArrive(VEHICLE* vehicle);

	private:

		std::mutex *lock_departed;
		std::mutex *lock_arrived;
		std::vector<VEHICLE*> departed_vehicles;
		std::vector<VEHICLE*> arrived_vehicles;
		float getCurrentTimeSeconds();
		int getCurrentTimeMilliseconds();
		std::vector<std::string> getDepartedVehicles();
		std::vector<std::string> getArrivedVehicles();
		int stepcnt;
	};
}

