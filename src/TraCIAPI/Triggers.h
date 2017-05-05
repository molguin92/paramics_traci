#pragma once
#include "programmer.h"
#include <string>
#include "Simulation.h"
#include <map>

namespace traci_api
{
    /* Triggered events (multiple causes: time, changing lanes, etc */
    class BaseTrigger
    {
    public:
        virtual ~BaseTrigger()
        {
        };
        virtual void handleTrigger() = 0;
        virtual bool repeat() = 0;
    };

    class SpeedChangeTrigger : public BaseTrigger
    {
    public:
        VEHICLE* vehicle;
        double speed;

        explicit SpeedChangeTrigger(VEHICLE* vehicle, double speed)
            : vehicle(vehicle), speed(speed)
        {
        }

        ~SpeedChangeTrigger() override
        {
        };

        void handleTrigger() override;
        bool repeat() override { return false; }
    };

    class SpeedSetTrigger : public SpeedChangeTrigger
    {
    public:
        SpeedSetTrigger(VEHICLE* vehicle, double speed)
            : SpeedChangeTrigger(vehicle, speed)
        {
        }

        ~SpeedSetTrigger() override
        {
        };

        bool repeat() override { return true; }
    };

    class LaneSetTrigger : public BaseTrigger
    {
    public:
        int target_lane;
        int end_time;
        VEHICLE* vehicle;

        LaneSetTrigger(VEHICLE* vhc, int target_lane, int duration);
        void handleTrigger() override;
        bool repeat() override;

        //private: 
        //    int orig_llane;
        //    int orig_hlane;
    };

    class RouteSetTrigger : public BaseTrigger
    {
        VEHICLE* vhc;
        std::map<LINK*, int> exit_map;

    public:
        RouteSetTrigger(VEHICLE* vhc, std::map<LINK*, int> exit_map) : vhc(vhc), exit_map(exit_map){}
        void handleTrigger() override;
        bool repeat() override { return !exit_map.empty(); };
    };

    /*class VehicleStopTrigger : public BaseTrigger
    {
    public:
        VEHICLE* vhc;
        LINK* lnk;
        double position;
        double sposition;
        int lane;
        double duration;
        bool done;

        VehicleStopTrigger(VEHICLE* vhc, LINK* lnk, double position, int lane, double duration, double sposition) : vhc(vhc), lnk(lnk), position(position), lane(lane), duration(duration), done(false), sposition(sposition)
        {
        }

        void handleTrigger() override;
        bool repeat() override { return !done; }

        ~VehicleStopTrigger() override
        {
        };
    };*/
}
