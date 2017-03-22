#include "VehicleManager.h"
#include <string>

/**
 * \brief Resets the internal temporary vectors for a new simulation timestep
 */
void traci_api::VehicleManager::reset()
{
	// clear temporary lists for a new timestep
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	departed_vehicles.clear();
	arrived_vehicles.clear();
}

/**
 * \brief Searchs the internal list of vehicles for a specific ID.
 * \param vid The vehicle ID to find.
 * \return A pointer to the corresponding Paramics Vehicle.
 */
VEHICLE* traci_api::VehicleManager::findVehicle(int vid)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	auto iterator = vehicles_in_sim.find(vid);
	if (iterator == vehicles_in_sim.end())
		throw NoSuchVHCError(std::to_string(vid));

	return iterator->second;
}

/**
 * \brief Signals the departure of a vehicle into the network.
 * \param vehicle A pointer to the Paramics Vehicle.
 */
void traci_api::VehicleManager::vehicleDepart(VEHICLE* vehicle)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	departed_vehicles.push_back(vehicle);
	vehicles_in_sim[qpg_VHC_uniqueID(vehicle)] = vehicle;
}

/**
 * \brief Signals the arrival of a vehicle from the network.
 * \param vehicle A pointer to the Paramics Vehicle.
 */
void traci_api::VehicleManager::vehicleArrive(VEHICLE* vehicle)
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	arrived_vehicles.push_back(vehicle);
	vehicles_in_sim.erase(qpg_VHC_uniqueID(vehicle));
}

/**
 * \brief Requests the list of recently departed vehicles.
 * \return A vector of strings containing every vehicle that has departed in the last timestep.
 */
std::vector<std::string> traci_api::VehicleManager::getDepartedVehicles()
{
	std::vector<std::string> ids;
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	for (VEHICLE* v : departed_vehicles)
		ids.push_back(std::to_string(qpg_VHC_uniqueID(v)));

	return ids;
}

/**
* \brief Requests the list of recently arrived vehicles.
* \return A vector of strings containing every vehicle that has arrived in the last timestep.
*/
std::vector<std::string> traci_api::VehicleManager::getArrivedVehicles()
{
	std::vector<std::string> ids;
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	for (VEHICLE* v : arrived_vehicles)
		ids.push_back(std::to_string(qpg_VHC_uniqueID(v)));

	return ids;
}

int traci_api::VehicleManager::getDepartedVehicleCount()
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	return departed_vehicles.size();
}

int traci_api::VehicleManager::getArrivedVehicleCount()
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	return arrived_vehicles.size();
}

int traci_api::VehicleManager::currentVehicleCount()
{
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	return vehicles_in_sim.size();
}

std::vector<std::string> traci_api::VehicleManager::getVehiclesInSim()
{
	std::vector<std::string> ids;
	std::lock_guard<std::mutex> lock(lock_vhc_lists);
	for (auto iterator : vehicles_in_sim)
		ids.push_back(std::to_string(iterator.first));

	return ids;
}

/**
 * \brief Requests the speed of a specific vehicle.
 * \param vid The ID of the vehicle.
 * \return The current speed.
 */
float traci_api::VehicleManager::getSpeed(int vid)
{
	return qpg_VHC_speed(this->findVehicle(vid));
}

/**
 * \brief Sets the speed of a specific vehicle.
 * \param vid The ID of the vehicle
 * \param speed The new value for the speed.
 */
void traci_api::VehicleManager::setSpeed(int vid, float speed)
{
	qps_VHC_speed(this->findVehicle(vid), speed);
}

/**
 * \brief Requests the 3-dimensional position of the vehicle in the simulation.
 * \param vid The ID of the vehicle.
 * \return A Vector3D object representing the position of the vehicle.
 */
PositionalData traci_api::VehicleManager::getPosition(int vid)
{
	float x;
	float y;
	float z;
	float b;
	float g; 

	VEHICLE* vhc = this->findVehicle(vid);
	LINK* lnk = qpg_VHC_link(vhc);

	qpg_POS_vehicle(vhc, lnk, &x, &y, &z, &b, &g);

	return PositionalData(x, y, z, b, g);
}

DimensionalData traci_api::VehicleManager::getDimensions(int vid)
{
	VEHICLE* vhc = this->findVehicle(vid);
	return DimensionalData(qpg_VHC_height(vhc), qpg_VHC_length(vhc), qpg_VHC_width(vhc));
}

std::string traci_api::VehicleManager::getRoadID(int vid)
{
	VEHICLE* vhc = this->findVehicle(vid);
	LINK* lnk = qpg_VHC_link(vhc);

	return qpg_LNK_name(lnk);
}

std::string traci_api::VehicleManager::getLaneID(int vid)
{
	VEHICLE* vhc = this->findVehicle(vid);
	LINK* lnk = qpg_VHC_link(vhc);

	return std::string(qpg_LNK_name(lnk)) + "." + std::to_string(qpg_VHC_lane(vhc));
}

int traci_api::VehicleManager::getLaneIndex(int vid)
{
	return qpg_VHC_lane(this->findVehicle(vid));
}

std::string traci_api::VehicleManager::getVehicleType(int vid)
{
	return std::to_string(qpg_VHC_type(this->findVehicle(vid)));
}



