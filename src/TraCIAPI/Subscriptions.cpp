#include "Subscriptions.h"
#include "VehicleManager.h"
#include "Constants.h"

uint8_t traci_api::VehicleSubscription::handleSubscription(tcpip::Storage& output)
{

    int time_status = checkTime();
    if (time_status < 0) // not yet
        return STATUS_TIMESTEPNOTREACHED;
    else if (time_status > 0) // expired
        return STATUS_EXPIRED;

    // prepare output
    output.writeString(objID);
    output.writeUnsignedByte(vars.size());

    // get ze vahriables
    tcpip::Storage temp;
    for (uint8_t sub_var : vars)
    {
        try {
            output.writeUnsignedByte(sub_var);
            VehicleManager::getInstance()->getVehicleVariable(objID, sub_var, temp);
            output.writeUnsignedByte(traci_api::STATUS_OK);
            output.writeStorage(temp);
            temp.reset();
        }
        catch ( std::runtime_error& e )
        {
            output.writeUnsignedByte(traci_api::STATUS_ERROR);
            output.writeUnsignedByte(VTYPE_STR);
            output.writeString(e.what());
        }
    }

    return STATUS_OK;
}
