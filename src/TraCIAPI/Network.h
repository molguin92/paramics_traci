#pragma once
#include <cstdint>
#include "storage.h"
#include "Exceptions.h"

namespace traci_api
{
    namespace  Network
    {
         const uint8_t VAR_LNK_LST = 0x00;
         const uint8_t VAR_LNK_CNT = 0x01;

         const uint8_t VAR_NDE_LST = 0x00;
         const uint8_t VAR_NDE_CNT = 0x01;
         const uint8_t VAR_NDE_POS = 0x42;
         const uint8_t VAR_NDE_SHP = 0x4e;

         void getLinkVariable(tcpip::Storage& input, tcpip::Storage& output) throw(traci_api::NoSuchLNKError);
         void getJunctionVariable(tcpip::Storage& input, tcpip::Storage& output) throw(traci_api::NoSuchNDEError);
    };
}
