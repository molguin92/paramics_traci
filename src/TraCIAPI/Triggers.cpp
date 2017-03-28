#include "Triggers.h"

void traci_api::LaneChangeTrigger::handleTrigger()
{
	qps_VHC_laneRange(this->vehicle, this->p_lane, this->p_lane);
}
