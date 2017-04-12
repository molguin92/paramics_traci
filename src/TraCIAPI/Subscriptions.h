#pragma once
#include <cstdint>
#include <vector>
#include "programmer.h"
#include "storage.h"
#include "Simulation.h"

namespace traci_api
{
    class Subscription
    {
    public:
        static const uint8_t STATUS_OK = 0x00;
        static const uint8_t STATUS_TIMESTEPNOTREACHED = 0x01;
        static const uint8_t STATUS_EXPIRED = 0x02;
        static const uint8_t STATUS_ERROR = 0xff;


        Subscription(std::string obj_id, int begin_time, int end_time, const std::vector<uint8_t>& vars):
            objID(obj_id),
            beginTime(begin_time),
            endTime(end_time),
            vars(vars)
        {
        }

        int checkTime() const
        {
            int current_time = Simulation::getInstance()->getCurrentTimeMilliseconds();
            if (beginTime > current_time) // begin time in the future
                return -1;
            else if (beginTime <= current_time && current_time <= endTime) // within range
                return 0;
            else // expired
                return 1;
        }

        virtual ~Subscription() {};
        virtual uint8_t handleSubscription(tcpip::Storage& output) { return STATUS_ERROR; };

    protected:
        std::string objID;
        int beginTime;
        int endTime;
        std::vector<uint8_t> vars;
    };

    class VehicleSubscription : Subscription
    {
    public:
        VehicleSubscription(std::string vhc_id, int begin_time, int end_time, const std::vector<uint8_t>& vars)
            : Subscription(vhc_id, begin_time, end_time, vars)
        {
        }

        ~VehicleSubscription() override {};
        uint8_t handleSubscription(tcpip::Storage& output) override;
    };
}
