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
    /* check if car is in correct road */
    if (lnk != qpg_VHC_link(vhc))
        return;
    else
    {
        qps_VHC_incident(vhc, -1, "TRACISTOP", -1, lane, position, duration, -1, -1);
    }
}
