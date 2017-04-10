#include "Triggers.h"

void traci_api::ResetLaneRangeTrigger::handleTrigger()
{
	qps_VHC_laneRange(this->vehicle, this->l_lane, this->h_lane);
}

void traci_api::SpeedChangeTrigger::handleTrigger()
{
	qps_VHC_speed(this->vehicle, this->speed);
	//qps_VHC_maxSpeed(this->vehicle, this->speed);
}

void traci_api::VehicleStopTrigger::handleTrigger()
{
	/* check if car is in correct road and lane */
	if (!(lnk == qpg_VHC_link(vhc) && lane == qpg_VHC_lane(vhc)))
		return;

	// TODO: FInish

	done = true;
}
