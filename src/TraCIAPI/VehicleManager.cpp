#include "VehicleManager.h"
#include "Constants.h"
#include <string>
#include "Simulation.h"

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
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
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

	switch (varID)
	{
	case STA_VHC_CHANGELANE:
		changeLane(input);
		break;
	case STA_VHC_SLOWDWN:
		slowDown(input);
		break;
	case STA_VHC_COLOUR:
		changeColour(input);
		break;

	case STA_VHC_STOP:
	case STA_VHC_RESUME:
	case STA_VHC_CHANGETARGET:
	case STA_VHC_SPEED:
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

void traci_api::VehicleManager::vehicleTimeStep(VEHICLE* vehicle)
{
	int vid = qpg_VHC_uniqueID(vehicle);
	{
		// update total internal vehicle list
		std::lock_guard<std::mutex> lock(vhc_lists_mutex);
		auto iterator = vehicles_in_sim.find(vid);
		if (iterator == vehicles_in_sim.end())
			throw std::runtime_error("Weird. Car in timestep but not added to list?");
	}

	// TODO: Check subs status here.
}

/**
 * \brief Searchs the internal list of vehicles for a specific ID.
 * \param vid The vehicle ID to find.
 * \return A pointer to the corresponding Paramics Vehicle.
 */
VEHICLE* traci_api::VehicleManager::findVehicle(int vid) throw(NoSuchVHCError)
{
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	auto iterator = vehicles_in_sim.find(vid);
	if (iterator == vehicles_in_sim.end())
		throw NoSuchVHCError(std::to_string(vid));

	return iterator->second;
}

/**
 * \brief Handles delayed triggers. For example, changing back to the original lane 
 * after a set time after a lane change command.
 */
void traci_api::VehicleManager::handleDelayedTriggers()
{
	std::lock_guard<std::mutex> lock(trigger_mutex);
	int current_time = Simulation::getInstance()->getCurrentTimeMilliseconds();
	auto itup = triggers.upper_bound(current_time); // first element with trigger time > current time
	std::vector<BaseTimeStepTrigger*> trash;

	for(auto it = triggers.begin(); it != itup; ++it)
	{
		it->second->handleTrigger();
		trash.push_back(it->second);
	}
	
	triggers.erase(triggers.begin(), itup); // delete all triggers up to the current time, so they don't get executed again.

	for (BaseTimeStepTrigger* trigger : trash) //deallocate memory
		delete(trigger);
}


/**
 * \brief Signals the departure of a vehicle into the network.
 * \param vehicle A pointer to the Paramics Vehicle.
 */
void traci_api::VehicleManager::vehicleDepart(VEHICLE* vehicle)
{
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	departed_vehicles.push_back(vehicle);
	vehicles_in_sim[qpg_VHC_uniqueID(vehicle)] = vehicle;
}

/**
 * \brief Signals the arrival of a vehicle from the network.
 * \param vehicle A pointer to the Paramics Vehicle.
 */
void traci_api::VehicleManager::vehicleArrive(VEHICLE* vehicle)
{
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
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
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
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
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	for (VEHICLE* v : arrived_vehicles)
		ids.push_back(std::to_string(qpg_VHC_uniqueID(v)));

	return ids;
}

int traci_api::VehicleManager::getDepartedVehicleCount()
{
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	return departed_vehicles.size();
}

int traci_api::VehicleManager::getArrivedVehicleCount()
{
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	return arrived_vehicles.size();
}

int traci_api::VehicleManager::currentVehicleCount()
{
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	return vehicles_in_sim.size();
}

std::vector<std::string> traci_api::VehicleManager::getVehiclesInSim()
{
	std::vector<std::string> ids;
	std::lock_guard<std::mutex> lock(vhc_lists_mutex);
	for (auto iterator : vehicles_in_sim)
		ids.push_back(std::to_string(iterator.first));

	return ids;
}

/**
 * \brief Requests the speed of a specific vehicle.
 * \param vid The ID of the vehicle.
 * \return The current speed in m/s.
 */
float traci_api::VehicleManager::getSpeed(int vid) throw(NoSuchVHCError)
{
	int mph = qpg_VHC_speed(this->findVehicle(vid)) * qpg_UTL_toExternalSpeed();
	return MPH2MS(mph);
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

void traci_api::VehicleManager::stopVehicle(tcpip::Storage& input) throw(NoSuchVHCError, std::runtime_error)
{
	/* stop message format
	 * 
	 * | type: compound | byte
	 * | items: 4 or 5	| int
	 * ------------------
	 * | type: string	| byte
	 * | edge id		| string
	 * ------------------
	 * | type: double	| byte
	 * | end position	| double
	 * ------------------
	 * | type: byte		| byte
	 * | lane index		| byte
	 * ------------------
	 * | type: int		| byte
	 * | duration(ms)	| int
	 * -----optional-----
	 * | type:  byte	| byte
	 * | stopflags		| byte 
	 * /

	/* extract message information and check types */
}

void traci_api::VehicleManager::changeLane(tcpip::Storage& input) throw(NoSuchVHCError, std::runtime_error)
{
	/* change lane message format
	*
	* | type: compound	| ubyte
	* | items: 2		| int
	* ------------------
	* | type: byte		| ubyte
	* | lane id			| ubyte
	* ------------------
	* | type: int		| ubyte
	* | duration		| int
	*/

	std::string vhcid = input.readString();

	if(DEBUG)
		printToParamics("Vehicle " + vhcid + " changing lanes.");

	if (input.readUnsignedByte() != VTYPE_COMPOUND || input.readInt() != 2 )
		throw std::runtime_error("Malformed TraCI message");

	int8_t new_lane = 0;
	if (!readTypeCheckingByte(input, new_lane))
		throw std::runtime_error("Malformed TraCI message");

	int duration = 0;
	if (!readTypeCheckingInt(input, duration))
		throw std::runtime_error("Malformed TraCI message");

	VEHICLE* vhc = findVehicle(std::stoi(vhcid));
	LINK* lnk = qpg_VHC_link(vhc);

	int n_lanes = qpg_LNK_lanes(lnk);
	
	if (new_lane > n_lanes)
		new_lane = n_lanes;
	else if (new_lane < 1)
		new_lane = 1;

	int current_lane = qpg_VHC_lane(vhc);
	int lane_range_h = qpg_VHC_laneHigh(vhc);
	int lane_range_l = qpg_VHC_laneLow(vhc);

	/* set new temporary lane range */
	qps_VHC_laneRange(vhc, new_lane, new_lane);

	/* force lane change */
	int diff = new_lane - current_lane;
	while(diff != 0)
	{
		if ( diff < 0)
		{
			qps_VHC_changeLane(vhc, -1);
			diff++;
		}
		else
		{
			qps_VHC_changeLane(vhc, +1);
			diff--;
		}
	}


	/* set trigger for resetting lane range */
	int trigger_time = duration + Simulation::getInstance()->getCurrentTimeMilliseconds();

	{
		std::lock_guard<std::mutex> lock(trigger_mutex);
		triggers.insert(std::make_pair(trigger_time, new ResetLaneRangeTrigger(vhc, lane_range_l, lane_range_h)));
	}
}

void traci_api::VehicleManager::slowDown(tcpip::Storage& input) throw(NoSuchVHCError, std::runtime_error)
{
	/* slow down message format
	*
	* | type: compound	| ubyte
	* | items: 2		| int
	* ------------------
	* | type: double	| ubyte
	* | speed			| ubyte
	* ------------------
	* | type: int		| ubyte
	* | duration		| int
	*/

	std::string vhcid = input.readString();
	VEHICLE* vhc = findVehicle(std::stoi(vhcid));

	if (DEBUG)
		traci_api::printToParamics("Vehicle " + vhcid + " slowing down.");

	if (input.readUnsignedByte() != VTYPE_COMPOUND || input.readInt() != 2)
		throw std::runtime_error("Malformed TraCI message");

	double target_speed = 0;
	if(!readTypeCheckingDouble(input, target_speed))
		throw std::runtime_error("Malformed TraCI message");

	int duration = 0;
	if(!readTypeCheckingInt(input, duration))
		throw std::runtime_error("Malformed TraCI message");

	if (duration == 0)
		throw std::runtime_error("Malformed TraCI message");

	/* calculate the number of timesteps required */
	/* note that speed from traci comes in m/s, whereas Paramics uses MPH*/
	int stepsize = qpg_CFG_timeStep() * 1000;
	int steps = abs(duration / stepsize);

	/* calculate the speed difference in each step */
	float ext_spd_factor = qpg_UTL_toExternalSpeed();
	float int_spd_factor = qpg_UTL_toInternalSpeed();

	/* do calculations in mph */
	target_speed = MS2MPH(target_speed);
	
	double current_speed = qpg_VHC_speed(vhc) * ext_spd_factor; // mph
	double speedstep = (current_speed - target_speed) / steps; // mph

	if (fabs(speedstep - 0) < 0.001)
		return; // do nothing, already at target speed

	/* do the first step, and schedule the rest with triggers */

	double new_speed = (current_speed - speedstep);
	qps_VHC_speed(vhc, new_speed * int_spd_factor);
	qps_VHC_maxSpeed(vhc, new_speed * int_spd_factor);
	steps--;
	int next_time = Simulation::getInstance()->getCurrentTimeMilliseconds();
	for(; steps > 0; steps--)
	{
		new_speed = new_speed - speedstep;
		next_time = next_time + stepsize;
		{
			std::lock_guard<std::mutex> lock(trigger_mutex);
			triggers.insert(std::make_pair(next_time, new SpeedChangeTrigger(vhc, new_speed * int_spd_factor)));
		}
	}
}

void traci_api::VehicleManager::changeColour(tcpip::Storage& input) throw(NoSuchVHCError, std::runtime_error)
{
	/* colour change message format
	 * 
	 * | vhc_id | ubyte | ubyte | ubyte | ubyte |
	 *				R		G		B		A
	*/

	std::string vhcid = input.readString();
	VEHICLE* vhc = findVehicle(std::stoi(vhcid));

	uint32_t hex = 0x000000;
	if (!readTypeCheckingColor(input, hex))
		throw std::runtime_error("Malformed TraCI message");

	/* change vehicle color */
	qps_DRW_vehicleColour(vhc, hex);
}
