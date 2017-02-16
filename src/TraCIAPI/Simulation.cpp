#include "Simulation.h"
#include <programmer.h>

traci_api::Simulation::Simulation()
{
}

traci_api::Simulation::~Simulation()
{
}

float traci_api::Simulation::getCurrentTimeSeconds()
{
	return qpg_CFG_simulationTime();
}

long traci_api::Simulation::getCurrentTimeMilliseconds()
{
	return this->getCurrentTimeSeconds() * 1000;
}
