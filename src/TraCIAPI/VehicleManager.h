#pragma once
#include "programmer.h"
#include <mutex>
#include <unordered_map>

namespace traci_api
{
	class VehicleManager
	{
	public:
		VehicleManager();
		~VehicleManager();
		void reset();

		void vehicleDepart(VEHICLE* vehicle);
		void vehicleArrive(VEHICLE* vehicle);

		std::vector<std::string> getDepartedVehicles();
		std::vector<std::string> getArrivedVehicles();

		float getSpeed(int vid);
		void setSpeed(int vid, float speed);


	private:
		std::mutex lock_vhc_lists;

		std::unordered_map<int, VEHICLE*> vehicles_in_sim;
		std::vector<VEHICLE*> departed_vehicles;
		std::vector<VEHICLE*> arrived_vehicles;
	};

	class NoSuchVHCError: public std::runtime_error
	{
	public:
		explicit NoSuchVHCError(const std::string& _Message)
			: runtime_error(_Message)
		{
		}

		explicit NoSuchVHCError(const char* _Message)
			: runtime_error(_Message)
		{
		}
	};
}

