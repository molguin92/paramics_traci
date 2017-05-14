#pragma once
#include "programmer.h"
#include <string>
#include "Simulation.h"
#include <unordered_map>

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
}
