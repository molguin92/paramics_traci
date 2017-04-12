#pragma once
#include <cstdint>
#include "storage.h"
#include "Exceptions.h"

namespace traci_api
{
    class Network
    {
    private:
        static const uint8_t VAR_LNK_LST = 0x00;
        static const uint8_t VAR_LNK_CNT = 0x01;

        static const uint8_t VAR_NDE_LST = 0x00;
        static const uint8_t VAR_NDE_CNT = 0x01;
        static const uint8_t VAR_NDE_POS = 0x42;
        static const uint8_t VAR_NDE_SHP = 0x4e;

    public:

        static void getLinkVariable(tcpip::Storage& input, tcpip::Storage& output) throw(traci_api::NoSuchLNKError);
        static void getJunctionVariable(tcpip::Storage& input, tcpip::Storage& output) throw(traci_api::NoSuchNDEError);
    };
}
