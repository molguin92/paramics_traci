#include "Triggers.h"

void traci_api::SpeedChangeTrigger::handleTrigger()
{
    qps_VHC_speed(this->vehicle, this->speed);
    //qps_VHC_maxSpeed(this->vehicle, this->speed);

    if (speed == 0 && !qpg_VHC_stopped(this->vehicle))
        qps_VHC_stopped(this->vehicle, PTRUE);
    else if (speed != 0 && qpg_VHC_stopped(this->vehicle))
        qps_VHC_stopped(this->vehicle, PFALSE);

}

void traci_api::LaneSetTrigger::handleTrigger()
{
    int t_lane = target_lane;

    int maxlanes = qpg_LNK_lanes(qpg_VHC_link(vehicle));
    if (t_lane > maxlanes)
        t_lane = maxlanes;
    else if (t_lane < 1)
        t_lane = 1;

    int current_lane = qpg_VHC_lane(vehicle);
    if (current_lane > t_lane)
        qps_VHC_laneChange(vehicle, -1);
    else if (current_lane < t_lane)
        qps_VHC_laneChange(vehicle, +1);
    else
        qps_VHC_laneChange(vehicle, 0);
}

bool traci_api::LaneSetTrigger::repeat()
{
    if (Simulation::getInstance()->getCurrentTimeMilliseconds() >= end_time)
        return false;
    else return true;
}
