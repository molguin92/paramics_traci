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
	default:
		return false;
	}

	return true;
}

void traci_api::Simulation::setVhcState(tcpip::Storage& state)
{
	uint8_t varID = state.readUnsignedByte();
	int vID = std::stoi(state.readString());
	uint8_t vType = state.readUnsignedByte();

	switch(varID)
	{
	case SET_VHCSPEED:
		if (vType != VTYPE_DOUBLE) throw std::runtime_error("Wrong VARTYPE for VHCSPEED.");
		else
			traci_api::VehicleManager::getInstance()->setSpeed(vID, static_cast<float>(state.readDouble()));

		break;

	default:
		throw std::runtime_error("No such Vehicle State Variable: " + std::to_string(varID));
	}
}
