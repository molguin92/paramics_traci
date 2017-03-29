#pragma once
#include "programmer.h"

namespace traci_api
{
	class BaseTimeStepTrigger
	{
	public:
		VEHICLE* vehicle;

		BaseTimeStepTrigger(VEHICLE* vehicle) : vehicle(vehicle)
		{
		}

		virtual ~BaseTimeStepTrigger() {};
		virtual void handleTrigger() = 0;
	};

	class ResetLaneRangeTrigger : public BaseTimeStepTrigger
	{
	public:
		int l_lane;
		int h_lane;

		ResetLaneRangeTrigger(VEHICLE* vehicle, int l_lane, int h_lane) : BaseTimeStepTrigger(vehicle), l_lane(l_lane), h_lane(h_lane)
		{
		}

		~ResetLaneRangeTrigger() override {};

		void handleTrigger() override;
	};

	class SpeedChangeTrigger : public BaseTimeStepTrigger
	{
		double speed;
	public:
		explicit SpeedChangeTrigger(VEHICLE* vehicle, double speed)
			: BaseTimeStepTrigger(vehicle), speed(speed)
		{
		}

		~SpeedChangeTrigger() override {};

		void handleTrigger() override;
	};
}
