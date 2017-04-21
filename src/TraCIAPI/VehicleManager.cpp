#include "VehicleManager.h"
#include "Constants.h"
#include <string>
#include "Simulation.h"
#include "Exceptions.h"

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

void traci_api::VehicleManager::packVehicleVariable(tcpip::Storage& input, tcpip::Storage& output) throw(NotImplementedError, std::runtime_error, NoSuchObjectError)
{
    uint8_t varID = input.readUnsignedByte();
    std::string s_vid = input.readString();

    //tcpip::Storage output;
    output.writeUnsignedByte(RES_GETVHCVAR);
    output.writeUnsignedByte(varID);
    output.writeString(s_vid);

    getVehicleVariable(s_vid, varID, output);

    //return output;
}

void traci_api::VehicleManager::getVehicleVariable(std::string vid, uint8_t varID, tcpip::Storage& output) throw(NotImplementedError, std::runtime_error, NoSuchObjectError)
{
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

    case VAR_VHC_SIGNALST:
        // the only signal state we can obtain from Paramics is brake light */
        {
            VEHICLE* v = findVehicle(vid);
            output.writeUnsignedByte(VTYPE_INT);
            if (qpg_VHC_braking(v))
                output.writeInt(0x08);
            else
                output.writeInt(0x00);
        }
        break;

        /* not implemented yet*/
    case VAR_VHC_ROUTE:
    case VAR_VHC_ROUTEIDX:
    case VAR_VHC_EDGES:
    case VAR_VHC_COLOR:
    case VAR_VHC_LANEPOS:
    case VAR_VHC_DIST:
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
    case STA_VHC_SPEED:
        setSpeed(input);
        break;

    case STA_VHC_MAXSPEED:
        setMaxSpeed(input);
        break;

    case STA_VHC_STOP:
    case STA_VHC_RESUME:
    case STA_VHC_CHANGETARGET:
    case STA_VHC_CHANGEROUTEID:
    case STA_VHC_CHANGEROUTE:
    case STA_VHC_CHANGEEDGETTIME:
    case STA_VHC_SIGNALSTATES:
    case STA_VHC_MOVETO:
    case STA_VHC_MOVETOXY:
    case STA_VHC_REROUTE:
    case STA_VHC_SPEEDMODE:
    case STA_VHC_SPEEDFACTOR:
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
VEHICLE* traci_api::VehicleManager::findVehicle(int vid) throw(NoSuchObjectError)
{
    std::lock_guard<std::mutex> lock(vhc_lists_mutex);
    auto iterator = vehicles_in_sim.find(vid);
    if (iterator == vehicles_in_sim.end())
        throw NoSuchObjectError(std::to_string(vid));

    return iterator->second;
}

VEHICLE* traci_api::VehicleManager::findVehicle(std::string vid) throw(NoSuchObjectError)
{
    return findVehicle(std::stoi(vid));
}

void traci_api::VehicleManager::packVhcTypesVariable(tcpip::Storage& input, tcpip::Storage& output) throw(std::runtime_error, NotImplementedError)
{
    uint8_t varID = input.readUnsignedByte();
    std::string type_id = input.readString();

    output.writeUnsignedByte(RES_GETVTPVAR);
    output.writeUnsignedByte(varID);
    output.writeString(type_id);

    int type_index = -1;
    if (varID != VARLST && varID != VARCNT)
    {
        try
        {
            type_index = types_index_map.at(type_id);
        }
        // ReSharper disable once CppEntityNeverUsed
        catch (std::out_of_range& e)
        {
            throw std::runtime_error("No such type: " + type_id);
        }
    }

    getVhcTypesVariable(type_index, varID, output);
}

void traci_api::VehicleManager::getVhcTypesVariable(int type_id, uint8_t varID, tcpip::Storage& output) throw(std::runtime_error, NotImplementedError)
{
    if (type_id < 0 && varID != VAR_VHC_LIST && varID != VAR_VHC_COUNT)
        throw std::runtime_error("Invalid type ID " + std::to_string(type_id) + " for variable " + std::to_string(varID));

    switch (varID)
    {
    case VAR_VHC_LIST:
        {
            std::vector<std::string> type_names;
            for (auto kv : types_index_map)
                type_names.push_back(kv.first);
            output.writeUnsignedByte(VTYPE_STRLST);
            output.writeStringList(type_names);
        }
        break;
    case VAR_VHC_COUNT:
        output.writeUnsignedByte(VTYPE_INT);
        output.writeInt(types_index_map.size());
        break;
    case VAR_VHC_LENGTH:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_length(type_id));
        break;
    case VAR_VHC_WIDTH:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_width(type_id));
        break;
    case VAR_VHC_HEIGHT:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_height(type_id));
        break;
    case VAR_VHC_ACCEL:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_acceleration(type_id));
        break;
    case VAR_VHC_VMAX:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_maxSpeed(type_id));
        break;
    case VAR_VHC_DECEL:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_deceleration(type_id));
        break;

    case VAR_VHC_TAU:
    case VAR_VHC_SIGMA:
    case VAR_VHC_SPDFACTOR:
    case VAR_VHC_SPEEDDEV:
    case VAR_VHC_VCLASS:
    case VAR_VHC_EMSCLASS:
    case VAR_VHC_SHAPE:
    case VAR_VHC_MINGAP:
    case VAR_VHC_COLOR:
    case VAR_VHC_MAXLATSPD:
    case VAR_VHC_LATGAP:
    case VAR_VHC_LATALIGN:
        throw NotImplementedError("Variable not implemented: " + std::to_string(varID));
    default:
        throw std::runtime_error("No such variable (" + std::to_string(varID) + ")");
    }
}

traci_api::VehicleManager::VehicleManager()
{
    int type_n = qpg_NET_vehicleTypes();
    for (int i = 1; i <= type_n; i++)
        types_index_map[qpg_VTP_name(i)] = i;
}

/**
 * \brief Handles delayed time_triggers. For example, changing back to the original lane 
 * after a set time after a lane change command.
 */
void traci_api::VehicleManager::handleDelayedTriggers()
{
    std::lock_guard<std::mutex> lock(time_trigger_mutex);
    int current_time = Simulation::getInstance()->getCurrentTimeMilliseconds();
    auto itup = time_triggers.upper_bound(current_time); // first element with trigger time > current time
    std::vector<BaseTrigger*> trash;

    for (auto it = time_triggers.begin(); it != itup;)
    {
        it->second->handleTrigger();

        /* delete all triggers handled, we don't want to see them again */
        delete(it->second);
        it = time_triggers.erase(it);
    }
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
float traci_api::VehicleManager::getSpeed(std::string vid) throw(NoSuchObjectError)
{
    //double mph = qpg_VHC_speed(this->findVehicle(vid)) * qpg_UTL_toExternalSpeed();
    //return KPH2MS(mph);
    return qpg_VHC_speed(findVehicle(vid));
}

/**
 * \brief Requests the 3-dimensional position of the vehicle in the simulation.
 * \param vid The ID of the vehicle.
 * \return A Vector3D object representing the position of the vehicle.
 */
PositionalData traci_api::VehicleManager::getPosition(std::string vid) throw(NoSuchObjectError)
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

DimensionalData traci_api::VehicleManager::getDimensions(std::string vid) throw(NoSuchObjectError)
{
    VEHICLE* vhc = this->findVehicle(vid);
    return DimensionalData(qpg_VHC_height(vhc), qpg_VHC_length(vhc), qpg_VHC_width(vhc));
}

std::string traci_api::VehicleManager::getRoadID(std::string vid) throw(NoSuchObjectError)
{
    VEHICLE* vhc = this->findVehicle(vid);
    LINK* lnk = qpg_VHC_link(vhc);

    return qpg_LNK_name(lnk);
}

std::string traci_api::VehicleManager::getLaneID(std::string vid) throw(NoSuchObjectError)
{
    VEHICLE* vhc = this->findVehicle(vid);
    LINK* lnk = qpg_VHC_link(vhc);

    return std::string(qpg_LNK_name(lnk)) + "." + std::to_string(qpg_VHC_lane(vhc));
}

int traci_api::VehicleManager::getLaneIndex(std::string vid) throw(NoSuchObjectError)
{
    return qpg_VHC_lane(this->findVehicle(vid));
}

std::string traci_api::VehicleManager::getVehicleType(std::string vid) throw(NoSuchObjectError)
{
    return std::to_string(qpg_VHC_type(this->findVehicle(vid)));
}

//void traci_api::VehicleManager::stopVehicle(tcpip::Storage& input) throw(NoSuchObjectError, NoSuchObjectError, std::runtime_error)
//{
//    ///* stop message format
//    // * 
//    // * | type: compound | byte
//    // * | items: 4 to 7	| int
//    // * ------------------
//    // * | type: string	| byte
//    // * | edge id		| string
//    // * ------------------
//    // * | type: double	| byte
//    // * | end position	| double
//    // * ------------------
//    // * | type: byte		| byte
//    // * | lane index		| byte
//    // * ------------------
//    // * | type: int		| byte
//    // * | duration(ms)	| int
//    // * -----optional-----
//    // * | type:  byte	| byte
//    // * | stopflags		| byte 
//    // * /
//
//    ///* extract message information and check types */
//    //std::string vhcid = input.readString();
//
//    //if (input.readUnsignedByte() != VTYPE_COMPOUND)
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //int c_items = input.readInt();
//    //if (c_items < 4 || c_items > 7)
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //std::string roadID = "";
//    //if (!readTypeCheckingString(input, roadID))
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //double position = 0.0;
//    //if (!readTypeCheckingDouble(input, position))
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //int8_t lane = 0;
//    //if (!readTypeCheckingByte(input, lane))
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //int duration = -1;
//    //if (!readTypeCheckingInt(input, duration))
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    ///*
//    // * optional flags:
//    // * 
//    // *	1 : parking
//    // *	2 : triggered
//    // *	4 : containerTriggered
//    // *	8 : busStop (Edge ID is re-purposed as busStop ID)
//    // *	16 : containerStop (Edge ID is re-purposed as containerStop ID)
//    // *	32 : chargingStation (Edge ID is re-purposed as chargingStation ID)
//    // *	64 : parkingArea (Edge ID is re-purposed as parkingArea ID)
//    // */
//
//    //bool parking = false,
//    //    triggered = false,
//    //    contTriggered = false,
//    //    busStop = false,
//    //    contStop = false,
//    //    chargStation = false,
//    //    parkingArea = false;
//
//    //if (c_items >= 5) // message includes flags
//    //{
//    //    int8_t flags = 0;
//    //    if (!readTypeCheckingByte(input, flags))
//    //        throw std::runtime_error("Malformed TraCI message");
//
//    //    parking = ((flags & 1) != 0);
//    //    triggered = ((flags & 2) != 0);
//    //    contTriggered = ((flags & 4) != 0);
//    //    busStop = ((flags & 8) != 0);
//    //    contStop = ((flags & 16) != 0);
//    //    chargStation = ((flags & 32) != 0);
//    //    parkingArea = ((flags & 64) != 0);
//    //}
//
//    //double start_position = position - POSITION_EPS;
//    //if (c_items >= 6 && !readTypeCheckingDouble(input, start_position))
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //int endtime = -1;
//    //if (c_items == 7 && !readTypeCheckingInt(input, endtime))
//    //    throw std::runtime_error("Malformed TraCI message");
//
//    //// TODO: special behavior for bus and container stops
//
//    //// check validity of parameters
//    //VEHICLE* vhc = findVehicle(std::stoi(vhcid));
//
//    //LINK* lnk = qpg_NET_link(&roadID[0u]);
//    //if (!lnk)
//    //    throw NoSuchObjectError(roadID);
//
//    //if (start_position < 0)
//    //    throw std::runtime_error("Position should be greater than 0");
//
//    //if (position < start_position)
//    //    throw std::runtime_error("Final position should be greater than start position");
//
//    //int n_lanes = qpg_LNK_lanes(lnk);
//    //if (lane < 1 || lane > n_lanes)
//    //    throw std::runtime_error("Lane index outside the range for this road. Number of lanes: " + std::to_string(n_lanes));
//}

void traci_api::VehicleManager::changeLane(tcpip::Storage& input) throw(NoSuchObjectError, std::runtime_error)
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


    debugPrint("Vehicle " + vhcid + " changing lanes.");

    if (input.readUnsignedByte() != VTYPE_COMPOUND || input.readInt() != 2)
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
    while (diff != 0)
    {
        if (diff < 0)
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
        std::lock_guard<std::mutex> lock(time_trigger_mutex);
        time_triggers.insert(std::make_pair(trigger_time, new ResetLaneRangeTrigger(vhc, lane_range_l, lane_range_h)));
    }
}

void traci_api::VehicleManager::slowDown(tcpip::Storage& input) throw(NoSuchObjectError, std::runtime_error)
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

    debugPrint("Vehicle " + vhcid + " slowing down.");

    if (input.readUnsignedByte() != VTYPE_COMPOUND || input.readInt() != 2)
        throw std::runtime_error("Malformed TraCI message");

    double target_speed = 0;
    if (!readTypeCheckingDouble(input, target_speed))
        throw std::runtime_error("Malformed TraCI message");

    int duration = 0;
    if (!readTypeCheckingInt(input, duration))
        throw std::runtime_error("Malformed TraCI message");

    if (duration == 0)
        throw std::runtime_error("Malformed TraCI message");

    /* calculate the number of timesteps required */
    /* note that speed from traci comes in m/s */
    int stepsize = qpg_CFG_timeStep() * 1000;
    int steps = abs(duration / stepsize);

    /* calculate the speed difference in each step */
    /* do calculations in m/s */

    double current_speed = qpg_VHC_speed(vhc); // m/s
    double speedstep = (current_speed - target_speed) / steps; // m/s

    if (fabs(speedstep - 0) < NUMERICAL_EPS)
        return; // do nothing, already at target speed

    /* do the first step, and schedule the rest with time_triggers */

    double new_speed = (current_speed - speedstep);
    qps_VHC_speed(vhc, new_speed);
    //qps_VHC_maxSpeed(vhc, new_speed * int_spd_factor);
    steps--;
    int next_time = Simulation::getInstance()->getCurrentTimeMilliseconds();
    for (; steps > 0; steps--)
    {
        new_speed = new_speed - speedstep;
        next_time = next_time + stepsize;
        {
            std::lock_guard<std::mutex> lock(time_trigger_mutex);
            time_triggers.insert(std::make_pair(next_time, new SpeedChangeTrigger(vhc, new_speed)));
        }
    }
}

void traci_api::VehicleManager::changeColour(tcpip::Storage& input) throw(NoSuchObjectError, std::runtime_error)
{
    /* colour change message format
     * 
     * | string | ubyte | ubyte | ubyte | ubyte | ubyte |
     *	 vhc_id	  Type		R		G		B		A
    */

    std::string vhcid = input.readString();
    VEHICLE* vhc = findVehicle(std::stoi(vhcid));

    uint32_t hex = 0x000000;
    if (!readTypeCheckingColor(input, hex))
        throw std::runtime_error("Malformed TraCI message");

    /* change vehicle color */
    qps_DRW_vehicleColour(vhc, hex);
}

void traci_api::VehicleManager::setSpeed(tcpip::Storage& input) throw(NoSuchObjectError, std::runtime_error)
{
    /* set speed message format
     * | string | ubyte | double |
     *   vhc_id	  Type	  speed
     */

    std::string vhcid = input.readString();
    VEHICLE* vhc = findVehicle(std::stoi(vhcid));

    double speed = 0;
    if (!readTypeCheckingDouble(input, speed))
        throw std::runtime_error("Malformed TraCI message");

    /* speed is in m/s */
    qps_VHC_speed(vhc, speed);
}

void traci_api::VehicleManager::setMaxSpeed(tcpip::Storage& input) throw(NoSuchObjectError, std::runtime_error)
{
    /* set maxspeed message format
    * | string | ubyte | double |
    *   vhc_id	  Type	  speed
    */

    std::string vhcid = input.readString();
    VEHICLE* vhc = findVehicle(std::stoi(vhcid));

    double speed = 0;
    if (!readTypeCheckingDouble(input, speed))
        throw std::runtime_error("Malformed TraCI message");

    /* speed is in m/s */
    qps_VHC_maxSpeed(vhc, speed);
}
