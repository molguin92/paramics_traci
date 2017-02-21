#include "Simulation.h"
#include <programmer.h>
#include "TraCIServer.h"
#include "Constants.h"

/*
 * This class abstracts a Paramics traffic simulation, providing helper methods for
 * interacting with the simulator itself.
 */

traci_api::Simulation::Simulation()
{
	stepcnt = 0;
	lock_departed = new std::mutex;
	lock_arrived = new std::mutex;
}

traci_api::Simulation::~Simulation()
{
	delete(lock_arrived);
	delete(lock_departed);
}

float traci_api::Simulation::getCurrentTimeSeconds()
{
	return qpg_CFG_simulationTime();
}

int traci_api::Simulation::getCurrentTimeMilliseconds()
{
	return static_cast<int>(this->getCurrentTimeSeconds() * 1000);
}

int traci_api::Simulation::runSimulation(uint32_t target_timems, tcpip::Storage& result_store)
{
	auto current_simtime = this->getCurrentTimeSeconds();
	auto target_simtime = target_timems / 1000.0;
	int steps_performed = 0;

	lock_departed->lock();
	departed_vehicles.clear();
	lock_departed->unlock();

	if (target_timems == 0)
	{
		if (DEBUG)
			TraCIServer::p_printf("Running one simulation step...");

		qps_GUI_runSimulation();
		steps_performed = 1;
	}
	else if (target_simtime > current_simtime)
	{
		if (DEBUG)
		{
			TraCIServer::p_printf("Running simulation up to target time: " + std::to_string(target_simtime));
			TraCIServer::p_printf("Current time: " + std::to_string(current_simtime));
		}

		while (target_simtime > current_simtime)
		{
			qps_GUI_runSimulation();
			steps_performed++;
			current_simtime = this->getCurrentTimeSeconds();

			if (DEBUG)
				TraCIServer::p_printf("Current time: " + std::to_string(current_simtime));
		}
	}
	else
	{
		if (DEBUG)
		{
			TraCIServer::p_printf("Invalid target simulation time: " + std::to_string(target_timems));
			TraCIServer::p_printf("Current simulation time: " + std::to_string(current_simtime));
			TraCIServer::p_printf("Doing nothing");
		}
	}

	// write subscription responses...
	result_store.writeInt(0);
	stepcnt += steps_performed;
	return steps_performed;
}

bool traci_api::Simulation::getVariable(uint8_t varID, tcpip::Storage& result_store)
{
	result_store.writeUnsignedByte(RES_GETSIMVAR);
	result_store.writeUnsignedByte(varID);
	result_store.writeString("");

	switch (varID)
	{
	case GET_SIMTIME:
		result_store.writeUnsignedByte(VTYPE_INT);
		result_store.writeInt(this->getCurrentTimeMilliseconds());
		break;
	case GET_DEPARTEDVHC_CNT:
		result_store.writeUnsignedByte(VTYPE_INT);
		lock_departed->lock();
		result_store.writeInt(departed_vehicles.size());
		lock_departed->unlock();
		break;
	case GET_DEPARTEDVHC_LST:
		result_store.writeUnsignedByte(VTYPE_STRLST);
		result_store.writeStringList(getDepartedVehicles());
		break;
	case GET_ARRIVEDVHC_CNT:
		result_store.writeUnsignedByte(VTYPE_INT);
		lock_arrived->lock();
		result_store.writeInt(arrived_vehicles.size());
		lock_arrived->unlock();
		break;
	case GET_ARRIVEDVHC_LST:
		result_store.writeUnsignedByte(VTYPE_STRLST);
		result_store.writeStringList(getArrivedVehicles());
		break;
	default:
		return false;
	}

	return true;
}

void traci_api::Simulation::vehicleDepart(VEHICLE* vehicle)
{
	lock_departed->lock();
	departed_vehicles.push_back(vehicle);
	lock_departed->unlock();
}

void traci_api::Simulation::vehicleArrive(VEHICLE* vehicle)
{
	lock_arrived->lock();
	arrived_vehicles.push_back(vehicle);
	lock_arrived->unlock();
}

std::vector<std::string> traci_api::Simulation::getDepartedVehicles()
{
	std::vector<std::string> vhcs;
	lock_departed->lock();
	for (VEHICLE* v : departed_vehicles)
		vhcs.push_back(std::to_string(qpg_VHC_uniqueID(v)));
	lock_departed->unlock();
	return vhcs;
}

std::vector<std::string> traci_api::Simulation::getArrivedVehicles()
{
	std::vector<std::string> vhcs;
	lock_arrived->lock();
	for (VEHICLE* v : arrived_vehicles)
		vhcs.push_back(std::to_string(qpg_VHC_uniqueID(v)));
	lock_arrived->unlock();
	return vhcs;
}