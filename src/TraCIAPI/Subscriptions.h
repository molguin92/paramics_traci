#pragma once
#include <cstdint>
#include <vector>
#include "programmer.h"
#include "storage.h"
#include "Simulation.h"

namespace traci_api
{
    class VariableSubscription
    {
    public:
        static const uint8_t RES_SUB_INDVAR = 0xe0;
        static const uint8_t RES_SUB_MULTVAR = 0xe1;
        static const uint8_t RES_SUB_TLIGHTVAR = 0xe2;
        static const uint8_t RES_SUB_LANEVAR = 0xe3;
        static const uint8_t RES_SUB_VHCVAR = 0xe4;
        static const uint8_t RES_SUB_VHCTYPEVAR = 0xe5;
        static const uint8_t RES_SUB_RTEVAR = 0xe6;
        static const uint8_t RES_SUB_POIVAR = 0xe7;
        static const uint8_t RES_SUB_POLVAR = 0xe8;
        static const uint8_t RES_SUB_JUNCTVAR = 0xe9;
        static const uint8_t RES_SUB_EDGEVAR = 0xea;
        static const uint8_t RES_SUB_SIMVAR = 0xeb;


        static const uint8_t STATUS_OK = 0x00;
        static const uint8_t STATUS_TIMESTEPNOTREACHED = 0x01;
        static const uint8_t STATUS_EXPIRED = 0x02;
        static const uint8_t STATUS_ERROR = 0xff;


        VariableSubscription(std::string obj_id, int begin_time, int end_time, std::vector<uint8_t> vars):
            objID(obj_id),
            beginTime(begin_time),
            endTime(end_time),
            vars(vars)
        {
        }

        int checkTime() const;

        virtual ~VariableSubscription() {};
        virtual uint8_t handleSubscription(tcpip::Storage& output, bool validate, std::string& errors) { return STATUS_ERROR; };

    protected:
        std::string objID;
        int beginTime;
        int endTime;
        std::vector<uint8_t> vars;
    };

    class VehicleVariableSubscription : public VariableSubscription
    {
    public:
        static const uint8_t STATUS_VHCNOTFOUND = 0xee;

        VehicleVariableSubscription(std::string vhc_id, int begin_time, int end_time, std::vector<uint8_t> vars)
            : VariableSubscription(vhc_id, begin_time, end_time, vars)
        {
        }

        ~VehicleVariableSubscription() override {};
        uint8_t handleSubscription(tcpip::Storage& output, bool validate, std::string& errors) override;
    };
}
