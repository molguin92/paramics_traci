#pragma once
#include "storage.h"
#include <unordered_map>


namespace traci_api
{
    class NotImplementedError;
    static const uint8_t VAR_LENGTH = 0X44;
    static const uint8_t VAR_VMAX = 0X41;
    static const uint8_t VAR_ACCEL = 0X46;
    static const uint8_t VAR_DECEL = 0X47;
    static const uint8_t VAR_WIDTH = 0X4D;
    static const uint8_t VAR_HEIGHT = 0XBC;

    static const uint8_t VAR_TAU = 0X48;
    static const uint8_t VAR_SIGMA = 0X5D;
    static const uint8_t VAR_SPDFACT = 0X5E;
    static const uint8_t VAR_SPDDEV = 0X5F;
    static const uint8_t VAR_VCLASS = 0X49;
    static const uint8_t VAR_EMCLASS = 0X4A;
    static const uint8_t VAR_SHAPE = 0X4B;
    static const uint8_t VAR_MINGAP = 0X4C;
    static const uint8_t VAR_COLOR = 0X45;
    static const uint8_t VAR_MAXLATSPD = 0XBA;
    static const uint8_t VAR_LATGAP = 0XBB;
    static const uint8_t VAR_LATALIGN = 0XB9;


    class VehicleTypes
    {
    private:
        static VehicleTypes* instance;

        VehicleTypes();
        ~VehicleTypes() {}

        std::unordered_map<std::string, int> index_map;


    public:
        /* prevent alternative instantiation */
        VehicleTypes(VehicleTypes const&) = delete;
        void operator=(VehicleTypes const&) = delete;

        static VehicleTypes* getInstance();
        void getVhcTypesVariable(tcpip::Storage& input, tcpip::Storage& output) throw(std::runtime_error, NotImplementedError);
    };
}
