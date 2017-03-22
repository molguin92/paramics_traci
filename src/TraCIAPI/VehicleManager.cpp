#include "VehicleManager.h"
#include <string>

traci_api::VehicleManager::VehicleManager()
{
}

traci_api::VehicleManager::~VehicleManager()
{
}

void traci_api::VehicleManager::reset()
{
	// clear temporary lists for a new timestep
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	departed_vehicles.clear();
	arrived_vehicles.clear();
}

void traci_api::VehicleManager::vehicleDepart(VEHICLE* vehicle)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	departed_vehicles.push_back(vehicle);
	vehicles_in_sim[qpg_VHC_uniqueID(vehicle)] = vehicle;
}

void traci_api::VehicleManager::vehicleArrive(VEHICLE* vehicle)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	arrived_vehicles.push_back(vehicle);
	vehicles_in_sim.erase(qpg_VHC_uniqueID(vehicle));
}

std::vector<std::string> traci_api::VehicleManager::getDepartedVehicles()
{
	std::vector<std::string> ids;

	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	for (VEHICLE* v : departed_vehicles)
		ids.push_back(std::to_string(qpg_VHC_uniqueID(v)));

	return ids;
}

std::vector<std::string> traci_api::VehicleManager::getArrivedVehicles()
{
	std::vector<std::string> ids;

	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	for (VEHICLE* v : arrived_vehicles)
		ids.push_back(std::to_string(qpg_VHC_uniqueID(v)));

	return ids;
}

float traci_api::VehicleManager::getSpeed(int vid)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	auto iterator = vehicles_in_sim.find(vid);
	if (iterator == vehicles_in_sim.end())
		throw NoSuchVHCError(std::to_string(vid));

	return qpg_VHC_speed(iterator->second);
}

void traci_api::VehicleManager::setSpeed(int vid, float speed)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	auto iterator = vehicles_in_sim.find(vid);
	if (iterator == vehicles_in_sim.end())
		throw NoSuchVHCError(std::to_string(vid));

	qps_VHC_speed(iterator->second, speed);
}
