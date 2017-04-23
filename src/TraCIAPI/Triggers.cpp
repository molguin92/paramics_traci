#include "Triggers.h"

void traci_api::ResetLaneRangeTrigger::handleTrigger()
{
    qps_VHC_laneRange(this->vehicle, this->l_lane, this->h_lane);
}

void traci_api::SpeedChangeTrigger::handleTrigger()
{
    qps_VHC_speed(this->vehicle, this->speed);
    //qps_VHC_maxSpeed(this->vehicle, this->speed);

    if (speed == 0 && !qpg_VHC_stopped(this->vehicle))
        qps_VHC_stopped(this->vehicle, PTRUE);
    else if (speed != 0 && qpg_VHC_stopped(this->vehicle))
        qps_VHC_stopped(this->vehicle, PFALSE);

}
