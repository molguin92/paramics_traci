#include "Simulation.h"
#include <programmer.h>
#include "TraCIServer.h"
#include "Constants.h"
#include "VehicleManager.h"

/*
 * This class abstracts a Paramics traffic simulation, providing helper methods for
 * interacting with the simulator itself.
 */

traci_api::Simulation* traci_api::Simulation::instance = nullptr;

traci_api::Simulation::Simulation(): stepcnt(0)
{
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

traci_api::Simulation* traci_api::Simulation::getInstance()
{
	if (instance == nullptr)
		instance = new Simulation();

	return instance;
}

void traci_api::Simulation::deleteInstance()
{
	if (instance != nullptr)
		delete(instance);

	instance = nullptr;
}

int traci_api::Simulation::runSimulation(uint32_t target_timems, tcpip::Storage& result_store)
{
	auto current_simtime = this->getCurrentTimeSeconds();
	auto target_simtime = target_timems / 1000.0;
	int steps_performed = 0;

	traci_api::VehicleManager::getInstance()->reset();

	if (target_timems == 0)
	{
		if (DEBUG)
			printToParamics("Running one simulation step...");

		qps_GUI_runSimulation();
		traci_api::VehicleManager::getInstance()->handleDelayedTriggers();
		steps_performed = 1;
	}
	else if (target_simtime > current_simtime)
	{
		if (DEBUG)
		{
			printToParamics("Running simulation up to target time: " + std::to_string(target_simtime));
			printToParamics("Current time: " + std::to_string(current_simtime));
		}

		while (target_simtime > current_simtime)
		{
			qps_GUI_runSimulation();
			steps_performed++;
			traci_api::VehicleManager::getInstance()->handleDelayedTriggers();

			current_simtime = this->getCurrentTimeSeconds();

			if (DEBUG)
				printToParamics("Current time: " + std::to_string(current_simtime));
		}
	}
	else
	{
		if (DEBUG)
		{
			printToParamics("Invalid target simulation time: " + std::to_string(target_timems));
			printToParamics("Current simulation time: " + std::to_string(current_simtime));
			printToParamics("Doing nothing");
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

	VehicleManager* vhcman = traci_api::VehicleManager::getInstance();

	switch (varID)
	{
	case GET_SIMTIME:
		result_store.writeUnsignedByte(VTYPE_INT);
		result_store.writeInt(this->getCurrentTimeMilliseconds());
		break;
	case GET_DEPARTEDVHC_CNT:
		result_store.writeUnsignedByte(VTYPE_INT);
		result_store.writeInt(vhcman->getDepartedVehicleCount());
		break;
	case GET_DEPARTEDVHC_LST:
		result_store.writeUnsignedByte(VTYPE_STRLST);
		result_store.writeStringList(vhcman->getDepartedVehicles());
		break;
	case GET_ARRIVEDVHC_CNT:
		result_store.writeUnsignedByte(VTYPE_INT);
		result_store.writeInt(vhcman->getArrivedVehicleCount());
		break;
	case GET_ARRIVEDVHC_LST:
		result_store.writeUnsignedByte(VTYPE_STRLST);
		result_store.writeStringList(vhcman->getArrivedVehicles());
		break;
	case GET_TIMESTEPSZ:
		result_store.writeUnsignedByte(VTYPE_INT);
		result_store.writeInt(static_cast<int>(qpg_CFG_timeStep() * 1000.0f));
		break;
	case GET_NETWORKBNDS:
		result_store.writeUnsignedByte(VTYPE_BOUNDBOX);
		{
			float llx, lly, urx, ury;
			qpg_POS_network(&llx, &lly, &urx, &ury);
			
			result_store.writeDouble(llx);
			result_store.writeDouble(lly);
			result_store.writeDouble(urx);
			result_store.writeDouble(ury);
		}
		break;
	default:
		return false;
	}

	return true;
}
