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

uint8_t traci_api::VariableSubscription::updateSubscription(uint8_t sub_type, std::string obj_id, int begin_time, int end_time, std::vector<uint8_t> vars, tcpip::Storage& result_store, std::string& errors)
{
    if (sub_type != this->sub_type || obj_id != objID)
        return STATUS_NOUPD;

    if (vars.size() == 0)
        return STATUS_UNSUB;

    // backup old values
    int old_start_time = this->beginTime;
    int old_end_time = this->endTime;
    std::vector<uint8_t> old_vars = this->vars;

    // set new values and try
    this->beginTime = begin_time;
    this->endTime = end_time;
    this->vars = vars;

    // validate
    uint8_t result = this->handleSubscription(result_store, true, errors);

    if (result == STATUS_EXPIRED)
        // if new time causes subscription to expire, just unsub
        return STATUS_UNSUB;
    else if (result != STATUS_OK)
    {
        // reset values
        this->beginTime = old_start_time;
        this->endTime = old_end_time;
        this->vars = old_vars;
    }

    return result;
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
        catch(NoSuchObjectError& e)
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
