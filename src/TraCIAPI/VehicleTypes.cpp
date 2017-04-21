#include "VehicleTypes.h"
#include <api_user.h>
#include "Constants.h"
#include "Exceptions.h"

namespace traci_api {
    class NotImplementedError;
}

traci_api::VehicleTypes* traci_api::VehicleTypes::instance = nullptr;

traci_api::VehicleTypes::VehicleTypes()
{
    int type_n = qpg_NET_vehicleTypes();
    for (int i = 1; i <= type_n; i++)
        index_map[qpg_VTP_name(i)] = i;
}

traci_api::VehicleTypes* traci_api::VehicleTypes::getInstance()
{
    if (!instance)
        instance = new VehicleTypes();

    return instance;
    // TODO: Delete instances
}

void traci_api::VehicleTypes::getVhcTypesVariable(tcpip::Storage& input, tcpip::Storage& output) throw(std::runtime_error, NotImplementedError)
{
    uint8_t varID = input.readUnsignedByte();
    std::string type_id = input.readString();

    output.writeUnsignedByte(RES_GETVTPVAR);
    output.writeUnsignedByte(varID);
    output.writeString(type_id);

    int type_index = -1;
    if (varID != VARLST && varID != VARCNT)
        type_index = index_map[type_id];

    switch (varID)
    {
    case VARLST:
        {
            std::vector<std::string> type_names;
            for (auto kv : index_map)
                type_names.push_back(kv.first);
            output.writeUnsignedByte(VTYPE_STRLST);
            output.writeStringList(type_names);
        }
        break;
    case VARCNT:
        output.writeUnsignedByte(VTYPE_INT);
        output.writeInt(index_map.size());
        break;
    case VAR_LENGTH:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_length(type_index));
        break;
    case VAR_WIDTH:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_width(type_index));
        break;
    case VAR_HEIGHT:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_height(type_index));
        break;
    case VAR_ACCEL:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_acceleration(type_index));
        break;
    case VAR_VMAX:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_maxSpeed(type_index));
        break;
    case VAR_DECEL:
        output.writeUnsignedByte(VTYPE_FLOAT);
        output.writeDouble(qpg_VTP_deceleration(type_index));
        break;

    case VAR_TAU:
    case VAR_SIGMA:
    case VAR_SPDFACT:
    case VAR_SPDDEV:
    case VAR_VCLASS:
    case VAR_EMCLASS:
    case VAR_SHAPE:
    case VAR_MINGAP:
    case VAR_COLOR:
    case VAR_MAXLATSPD:
    case VAR_LATGAP:
    case VAR_LATALIGN:
        throw NotImplementedError("Variable not implemented: " + std::to_string(varID));
    default:
        throw std::runtime_error("No such variable (" + std::to_string(varID) + ")");
    }
}
