#include "Subscriptions.h"
#include "VehicleManager.h"
#include "Constants.h"
#include "Exceptions.h"

int traci_api::VariableSubscription::checkTime() const
{
    int current_time = Simulation::getInstance()->getCurrentTimeMilliseconds();
    if (beginTime > current_time) // begin time in the future
        return -1;
    else if (beginTime <= current_time && current_time <= endTime) // within range
        return 0;
    else // expired
        return 1;
}

uint8_t traci_api::VehicleVariableSubscription::handleSubscription(tcpip::Storage& output, bool validate, std::string& errors)
{

    int time_status = checkTime();
    if (!validate && time_status < 0) // not yet (skip this check if validating, duh)
        return STATUS_TIMESTEPNOTREACHED;
    else if (time_status > 0) // expired
        return STATUS_EXPIRED;

    // prepare output
    output.writeUnsignedByte(RES_SUB_VHCVAR);
    output.writeString(objID);
    output.writeUnsignedByte(vars.size());

    bool result_errors = false;

    // get ze vahriables
    tcpip::Storage temp;
    for (uint8_t sub_var : vars)
    {
        try {
            output.writeUnsignedByte(sub_var);
            VehicleManager::getInstance()->getVehicleVariable(objID, sub_var, temp);
            output.writeUnsignedByte(traci_api::STATUS_OK);
            output.writeStorage(temp);
        }
        // ReSharper disable once CppEntityNeverUsed
        catch(NoSuchVHCError& e)
        {
            errors = "Vehicle " + objID + " not found in simulation.";
            return STATUS_VHCNOTFOUND;
        }
        catch ( std::runtime_error& e )
        {
            result_errors = true;
            output.writeUnsignedByte(traci_api::STATUS_ERROR);
            output.writeUnsignedByte(VTYPE_STR);
            output.writeString(e.what());
            errors += std::string(e.what()) + "; ";
        }

        temp.reset();
    }

    if (validate && result_errors)
        return STATUS_ERROR;
    else
        return STATUS_OK;
}

uint8_t traci_api::SimulationVariableSubscription::handleSubscription(tcpip::Storage& output, bool validate, std::string& errors)
{

    int time_status = checkTime();
    if (!validate && time_status < 0) // not yet (skip this check if validating, duh)
        return STATUS_TIMESTEPNOTREACHED;
    else if (time_status > 0) // expired
        return STATUS_EXPIRED;

    // prepare output
    output.writeUnsignedByte(RES_SUB_SIMVAR);
    output.writeString(objID);
    output.writeUnsignedByte(vars.size());

    bool result_errors = false;

    // get ze vahriables
    tcpip::Storage temp;
    for (uint8_t sub_var : vars)
    {
        try 
        {
            output.writeUnsignedByte(sub_var);
            Simulation::getInstance()->getSimulationVariable(sub_var, temp);
            output.writeUnsignedByte(traci_api::STATUS_OK);
            output.writeStorage(temp);
        }
        catch (std::runtime_error& e)
        {
            result_errors = true;
            output.writeUnsignedByte(traci_api::STATUS_ERROR);
            output.writeUnsignedByte(VTYPE_STR);
            output.writeString(e.what());
            errors += std::string(e.what()) + "; ";
        }

        temp.reset();
    }

    if (validate && result_errors)
        return STATUS_ERROR;
    else
        return STATUS_OK;
}
