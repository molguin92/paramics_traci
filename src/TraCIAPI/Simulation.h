#pragma once

namespace traci_api
{
	class Simulation
	{
	public:
		Simulation();
		~Simulation();

		float getCurrentTimeSeconds();
		long getCurrentTimeMilliseconds();
	};
}

