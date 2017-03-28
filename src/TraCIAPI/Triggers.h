#pragma once
#include "programmer.h"

namespace traci_api
{
	class BaseTimeStepTrigger
	{
	public:
		VEHICLE* vehicle;
		int timestep_ms;

		BaseTimeStepTrigger(VEHICLE* vehicle, int timestep) : vehicle(vehicle), timestep_ms(timestep)
		{
		}

		virtual ~BaseTimeStepTrigger() = 0;
		virtual void handleTrigger() = 0;
	};

	class LaneChangeTrigger : public BaseTimeStepTrigger
	{
	public:
		int p_lane;

		LaneChangeTrigger(VEHICLE* vehicle, int timestep, int p_lane) : BaseTimeStepTrigger(vehicle, timestep), p_lane(p_lane)
		{
		}

		~LaneChangeTrigger() override {};

		void handleTrigger() override;
	};
}
