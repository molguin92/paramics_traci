#include "Simulation.h"
#include <programmer.h>
#include "TraCIServer.h"
#include "Constants.h"

traci_api::Simulation::Simulation()
{
	stepcnt = 0;
}

traci_api::Simulation::~Simulation()
{
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

	lock->lock();
	released_vehicles.clear();
	lock->unlock();

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
	case GET_RELEASEDVHC_CNT:
		lock->lock();
		result_store.writeUnsignedByte(VTYPE_INT);
		result_store.writeInt(released_vehicles.size());
		lock->unlock();
		break;
	default:
		return false;
	}

	return true;
}

void traci_api::Simulation::releaseVehicle(VEHICLE* vehicle)
{
	lock->lock();
	released_vehicles.push_back(vehicle);
	lock->unlock();
}
