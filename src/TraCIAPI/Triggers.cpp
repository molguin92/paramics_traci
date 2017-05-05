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

traci_api::LaneSetTrigger::LaneSetTrigger(VEHICLE* vhc, int target_lane, int duration) : target_lane(target_lane), vehicle(vhc)
{
    end_time = Simulation::getInstance()->getCurrentTimeMilliseconds() + duration;
    //orig_hlane = qpg_VHC_laneHigh(vhc);
    //orig_llane = qpg_VHC_laneLow(vhc);
}


void traci_api::LaneSetTrigger::handleTrigger()
{
    int t_lane = target_lane;

    int maxlanes = qpg_LNK_lanes(qpg_VHC_link(vehicle));
    if (t_lane > maxlanes)
        t_lane = maxlanes;
    else if (t_lane < 1)
        t_lane = 1;

    // additionally, set lane range for better lane change behavior
    //qps_VHC_laneRange(vehicle, t_lane, t_lane);

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
    {
        // reset lane range
        //qps_VHC_laneRange(vehicle, orig_llane, orig_hlane);
        return false;
    }
    else return true;
}

void traci_api::RouteSetTrigger::handleTrigger()
{
    LINK* current_link = qpg_VHC_link(vhc);
    int next_exit = exit_map[current_link];
    qps_VHC_nextlink(vhc, next_exit);

    exit_map.erase(current_link);
}
