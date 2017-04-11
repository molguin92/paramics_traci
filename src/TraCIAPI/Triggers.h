#pragma once
#include "programmer.h"
#include <string>

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

    class ResetLaneRangeTrigger : public BaseTrigger
    {
    public:
        VEHICLE* vehicle;
        int l_lane;
        int h_lane;

        ResetLaneRangeTrigger(VEHICLE* vehicle, int l_lane, int h_lane) : vehicle(vehicle), l_lane(l_lane), h_lane(h_lane)
        {
        }

        ~ResetLaneRangeTrigger() override
        {
        };

        void handleTrigger() override;
        bool repeat() override { return false; }
    };

    class SpeedChangeTrigger : public BaseTrigger
    {
        VEHICLE* vehicle;
        double speed;
    public:
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

    class VehicleStopTrigger : public BaseTrigger
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
    };
}
