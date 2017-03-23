#include "VehicleManager.h"
#include "Constants.h"
#include <string>

/* null singleton */
traci_api::VehicleManager* traci_api::VehicleManager::instance = nullptr;

traci_api::VehicleManager* traci_api::VehicleManager::getInstance()
{
	if (traci_api::VehicleManager::instance == nullptr)
		traci_api::VehicleManager::instance = new VehicleManager();

	return traci_api::VehicleManager::instance;
}

void traci_api::VehicleManager::deleteInstance()
{
	if (traci_api::VehicleManager::instance != nullptr)
		delete(traci_api::VehicleManager::instance);

	traci_api::VehicleManager::instance = nullptr;
}

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

void traci_api::VehicleManager::getVehicleVariable(tcpip::Storage& input, tcpip::Storage& output) throw(NotImplementedError, std::runtime_error, NoSuchVHCError)
{
	uint8_t varID = input.readUnsignedByte();
	int vid;
	std::string s_vid;

	if (varID != VAR_VHC_LIST && varID != VAR_VHC_COUNT)
	{
		s_vid = input.readString();
		vid = std::stoi(s_vid);
	}
	else
	{
		s_vid = "";
		vid = 0;
	}

	//tcpip::Storage output;
	output.writeUnsignedByte(RES_GETVHCVAR);
	output.writeUnsignedByte(varID);
	output.writeString(s_vid);

	switch (varID)
	{
	case VAR_VHC_LIST:
		output.writeUnsignedByte(VTYPE_STRLST);
		output.writeStringList(getVehiclesInSim());
		break;

	case VAR_VHC_COUNT:
		output.writeUnsignedByte(VTYPE_INT);
		output.writeInt(currentVehicleCount());
		break;

	case VAR_VHC_SPEED:
		output.writeUnsignedByte(VTYPE_DOUBLE);
		output.writeDouble(getSpeed(vid));
		break;

	case VAR_VHC_POS:
		output.writeUnsignedByte(VTYPE_POSITION);
		{
			PositionalData pos = getPosition(vid);
			output.writeDouble(pos.x);
			output.writeDouble(pos.y);
		}
		break;

	case VAR_VHC_POS3D:
		output.writeUnsignedByte(VTYPE_POSITION3D);
		{
			PositionalData pos = getPosition(vid);
			output.writeDouble(pos.x);
			output.writeDouble(pos.y);
			output.writeDouble(pos.z);
		}
		break;

	case VAR_VHC_ANGLE:
		output.writeUnsignedByte(VTYPE_DOUBLE);
		output.writeDouble(getPosition(vid).bearing);
		break;

	case VAR_VHC_ROAD:
		output.writeUnsignedByte(VTYPE_STR);
		output.writeString(getRoadID(vid));
		break;

	case VAR_VHC_LANE:
		output.writeUnsignedByte(VTYPE_STR);
		output.writeString(getLaneID(vid));
		break;

	case VAR_VHC_LANEIDX:
		output.writeUnsignedByte(VTYPE_INT);
		output.writeInt(getLaneIndex(vid));
		break;

	case VAR_VHC_TYPE:
		output.writeUnsignedByte(VTYPE_STR);
		output.writeString(getVehicleType(vid));
		break;

	case VAR_VHC_LENGTH:
		output.writeUnsignedByte(VTYPE_DOUBLE);
		output.writeDouble(getDimensions(vid).length);
		break;

	case VAR_VHC_WIDTH:
		output.writeUnsignedByte(VTYPE_DOUBLE);
		output.writeDouble(getDimensions(vid).width);
		break;

	case VAR_VHC_HEIGHT:
		output.writeUnsignedByte(VTYPE_DOUBLE);
		output.writeDouble(getDimensions(vid).height);
		break;

	case VAR_VHC_SLOPE:
		output.writeUnsignedByte(VTYPE_DOUBLE);
		output.writeDouble(getPosition(vid).gradient);
		break;

		/* not implemented yet*/
	case VAR_VHC_ROUTE:
	case VAR_VHC_ROUTEIDX:
	case VAR_VHC_EDGES:
	case VAR_VHC_COLOR:
	case VAR_VHC_LANEPOS:
	case VAR_VHC_DIST:
	case VAR_VHC_SIGNALST:
	case VAR_VHC_CO2:
	case VAR_VHC_CO:
	case VAR_VHC_HC:
	case VAR_VHC_PMX:
	case VAR_VHC_NOX:
	case VAR_VHC_FUELCONS:
	case VAR_VHC_NOISE:
	case VAR_VHC_ELECCONS:
	case VAR_VHC_BESTLANES:
	case VAR_VHC_STOPSTATE:
	case VAR_VHC_VMAX:
	case VAR_VHC_ACCEL:
	case VAR_VHC_DECEL:
	case VAR_VHC_TAU:
	case VAR_VHC_SIGMA:
	case VAR_VHC_SPDFACTOR:
	case VAR_VHC_SPEEDDEV:
	case VAR_VHC_VCLASS:
	case VAR_VHC_EMSCLASS:
	case VAR_VHC_SHAPE:
	case VAR_VHC_MINGAP:
	case VAR_VHC_WAITTIME:
	case VAR_VHC_NEXTTLS:
	case VAR_VHC_SPEEDMODE:
	case VAR_VHC_ALLOWEDSPD:
	case VAR_VHC_LINE:
	case VAR_VHC_PNUMBER:
	case VAR_VHC_VIAEDGES:
	case VAR_VHC_NONTRACISPD:
	case VAR_VHC_VALIDROUTE:
		throw NotImplementedError("Vehicle Variable not implemented: " + std::to_string(varID));
	default:
		throw std::runtime_error("No such variable: " + std::to_string(varID));
	}

	//return output;
}

void traci_api::VehicleManager::setVehicleState(tcpip::Storage& input)
{
	uint8_t varID = input.readUnsignedByte();
	uint8_t varType = input.readUnsignedByte();

	switch (varID)
	{
	case STA_VHC_STOP:
	case STA_VHC_CHANGELANE:
	case STA_VHC_SLOWDWN:
	case STA_VHC_RESUME:
	case STA_VHC_CHANGETARGET:
	case STA_VHC_SPEED:
	case STA_VHC_COLOR:
	case STA_VHC_CHANGEROUTEID:
	case STA_VHC_CHANGEROUTE:
	case STA_VHC_CHANGEEDGETTIME:
	case STA_VHC_SIGNALSTATES:
	case STA_VHC_MOVETO:
	case STA_VHC_MOVETOXY:
	case STA_VHC_REROUTE:
	case STA_VHC_SPEEDMODE:
	case STA_VHC_SPEEDFACTOR:
	case STA_VHC_MAXSPEED:
	case STA_VHC_CHANGELANEMODE:
	case STA_VHC_ADD:
	case STA_VHC_ADDFULL:
	case STA_VHC_REMOVE:
	case STA_VHC_LENGTH:
	case STA_VHC_VHCCLASS:
	case STA_VHC_EMSCLASS:
	case STA_VHC_WIDTH:
	case STA_VHC_HEIGHT:
	case STA_VHC_MINGAP:
	case STA_VHC_SHAPECLASS:
	case STA_VHC_ACC:
	case STA_VHC_DEC:
	case STA_VHC_IMPERFECTION:
	case STA_VHC_TAU:
	case STA_VHC_TYPE:
	case STA_VHC_VIA:
		throw NotImplementedError("Vehicle State change not implemented: " + std::to_string(varID));
	default:
		throw std::runtime_error("No such State change: " + std::to_string(varID));
	}
}

/**
 * \brief Searchs the internal list of vehicles for a specific ID.
 * \param vid The vehicle ID to find.
 * \return A pointer to the corresponding Paramics Vehicle.
 */
VEHICLE* traci_api::VehicleManager::findVehicle(int vid) throw(NoSuchVHCError)
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
float traci_api::VehicleManager::getSpeed(int vid) throw(NoSuchVHCError)
{
	return qpg_VHC_speed(this->findVehicle(vid));
}

/**
 * \brief Sets the speed of a specific vehicle.
 * \param vid The ID of the vehicle
 * \param speed The new value for the speed.
 */
void traci_api::VehicleManager::setSpeed(int vid, float speed) throw(NoSuchVHCError)
{
	qps_VHC_speed(this->findVehicle(vid), speed);
}

/**
 * \brief Requests the 3-dimensional position of the vehicle in the simulation.
 * \param vid The ID of the vehicle.
 * \return A Vector3D object representing the position of the vehicle.
 */
PositionalData traci_api::VehicleManager::getPosition(int vid) throw(NoSuchVHCError)
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

DimensionalData traci_api::VehicleManager::getDimensions(int vid) throw(NoSuchVHCError)
{
	VEHICLE* vhc = this->findVehicle(vid);
	return DimensionalData(qpg_VHC_height(vhc), qpg_VHC_length(vhc), qpg_VHC_width(vhc));
}

std::string traci_api::VehicleManager::getRoadID(int vid) throw(NoSuchVHCError)
{
	VEHICLE* vhc = this->findVehicle(vid);
	LINK* lnk = qpg_VHC_link(vhc);

	return qpg_LNK_name(lnk);
}

std::string traci_api::VehicleManager::getLaneID(int vid) throw(NoSuchVHCError)
{
	VEHICLE* vhc = this->findVehicle(vid);
	LINK* lnk = qpg_VHC_link(vhc);

	return std::string(qpg_LNK_name(lnk)) + "." + std::to_string(qpg_VHC_lane(vhc));
}

int traci_api::VehicleManager::getLaneIndex(int vid) throw(NoSuchVHCError)
{
	return qpg_VHC_lane(this->findVehicle(vid));
}

std::string traci_api::VehicleManager::getVehicleType(int vid) throw(NoSuchVHCError)
{
	return std::to_string(qpg_VHC_type(this->findVehicle(vid)));
}
