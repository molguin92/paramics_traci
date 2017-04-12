#include "Triggers.h"
#include <limits>

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

    // first case: we're in the same link and lane as the stop
    if (lnk == qpg_VHC_link(vhc));

}
