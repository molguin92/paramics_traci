#include "Triggers.h"

void traci_api::ResetLaneRangeTrigger::handleTrigger()
{
	qps_VHC_laneRange(this->vehicle, this->l_lane, this->h_lane);
}
