#include "TraCIServer.h"
#include "programmer.h"
#include <thread>
#include "storage.h"
#include "Constants.h"
#include "VehicleManager.h"
#include "Exceptions.h"
#include "Network.h"
#include "Subscriptions.h"
#include <windows.h>

/*
 * This class abstracts a server for the TraCI protocol.
 * It binds to and listens on a port for incoming TraCI commands, and replies with the appropiate data.
 */


/**
 * \brief Standard constructor.
 * \param port The port on which the server should listen for incoming requests.
 */
traci_api::TraCIServer::TraCIServer(int port): ssocket(port), running(false), port(port)
{
    ssocket.set_blocking(true);
}


traci_api::TraCIServer::~TraCIServer()
{
    VehicleManager::deleteInstance();
    debugPrint("Server succesfully shut down");
}

/**
 * \brief Starts this instance, binding it to a port and awaiting connections. This method is blocking, and as such should be run in a separate thread.
 */
void traci_api::TraCIServer::run()
{
    running = true;
    std::string version_str = "Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion());
    infoPrint(version_str);
    debugPrint("Simulation start time: " + std::to_string(qpg_CFG_simulationTime()));
    infoPrint("Awaiting connections on port " + std::to_string(port));
    ssocket.accept();
    infoPrint("Accepted connection");
    this->waitForCommands();
}

/**
 * \brief Closes the socket, severing all connections.
 */
void traci_api::TraCIServer::close()
{
    debugPrint("Closing connections");
    running = false;
    ssocket.close();
}


/**
 * \brief Waits for incomming commands on the TCP socket.
 */
void traci_api::TraCIServer::waitForCommands()
{
    tcpip::Storage incoming; // the whole incoming message
    tcpip::Storage cmdStore; // individual commands in the message

    debugPrint("Waiting for incoming commands from TraCI client...");

    /* While the connection is open, receive commands from the client */
    while (running && ssocket.receiveExact(incoming))
    {
        auto msize = incoming.size();

        debugPrint("Got message of length " + std::to_string(msize));
        debugPrint("Incoming: " + incoming.hexDump());


        /* Multiple commands may arrive at once in one message, 
         * divide them into multiple storages for easy handling */
        while (msize > 0 && incoming.valid_pos())
        {
            uint8_t cmdlen = incoming.readUnsignedByte();
            cmdStore.writeUnsignedByte(cmdlen);

            debugPrint("Got command of length " + std::to_string(cmdlen));


            for (uint8_t i = 0; i < cmdlen - 1; i++)
                cmdStore.writeUnsignedByte(incoming.readUnsignedByte());

            this->parseCommand(cmdStore);
            cmdStore.reset();
        }

        this->sendResponse();
        incoming.reset();
        outgoing.reset();
        debugPrint("------ waiting for commands ------");
    }
}

/**
 * \brief Parses an incoming command according to the TraCI protocol specifications.
 * \param storage A tcpip::Storage object which contains a single TraCI command.
 */
void traci_api::TraCIServer::parseCommand(tcpip::Storage& storage)
{
    debugPrint("Parsing command");

    uint8_t cmdLen = storage.readUnsignedByte();
    uint8_t cmdId = storage.readUnsignedByte();
    tcpip::Storage state;

    debugPrint("Command length: " + std::to_string(cmdLen));
    debugPrint("Command ID: " + std::to_string(cmdId));

    if (cmdId >= CMD_SUB_INDVAR && cmdId <= CMD_SUB_SIMVAR)
    {
        // subscription
        // | begin Time | end Time | Object ID | Variable Number | The list of variables to return

        debugPrint("Subscribing to " + std::to_string(cmdId));


        int btime = storage.readInt();
        debugPrint("Start time: " + std::to_string(btime));

        int etime = storage.readInt();
        debugPrint("End time: " + std::to_string(etime));

        std::string oID = storage.readString();
        debugPrint("Object ID: " + oID);

        int varN = storage.readUnsignedByte();
        debugPrint("N Vars: " + std::to_string(varN));

        std::vector<uint8_t> vars;
        std::string vars_s = "";

        for (int i = 0; i < varN; i++)
        {
            uint8_t vid = storage.readUnsignedByte();
            vars.push_back(vid);
            vars_s = vars_s + std::to_string(vid) + " ";
        }

        debugPrint("Vars: " + vars_s);
        addSubscription(cmdId, oID, btime, etime, vars);
    }
    else
    {
        switch (cmdId)
        {
        case CMD_GETVERSION:

            debugPrint("Got CMD_GETVERSION");
            this->writeVersion();
            break;

        case CMD_SIMSTEP:

            debugPrint("Got CMD_SIMSTEP");

            this->cmdSimStep(storage.readInt());
            break;

        case CMD_SHUTDOWN:

            debugPrint("Got CMD_SHUTDOWN");
            this->cmdShutDown();
            break;

        case CMD_GETSIMVAR:

            debugPrint("Got CMD_GETSIMVAR");
            this->cmdGetSimVar(storage.readUnsignedByte());
            break;

        case CMD_SETVHCSTATE:

            debugPrint("Got CMD_SETVHCSTATE");
            this->cmdSetVhcState(storage);
            break;

        case CMD_GETVHCVAR:

            debugPrint("Got CMD_GETVHCVAR");
            this->cmdGetVhcVar(storage);
            break;

        case CMD_GETLNKVAR:
        case CMD_GETNDEVAR:

            debugPrint("Got CMD_GETLNKVAR/CMD_GETNDEVAR");
            this->cmdGetNetworkVar(storage, cmdId);
            break;

        case CMD_GETPOLVAR:
            debugPrint("Got CMD_GETPOLVAR");
            this->cmdGetPolygonVar(storage);            
            break;

        default:

            debugPrint("Command not implemented!");

            writeStatusResponse(cmdId, STATUS_NIMPL, "Method not implemented.");
        }
    }
}

/**
 * \brief Writes a status respond to a specific command in the outgoing TraCI message to be sent back to the client.
 * \param cmdId The command to respond to.
 * \param cmdStatus The status response.
 * \param description A std::string describing the result.
 */
void traci_api::TraCIServer::writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description)
{
    debugPrint("Writing status response " + std::to_string(cmdStatus) + " for command " + std::to_string(cmdId));

    outgoing.writeUnsignedByte(1 + 1 + 1 + 4 + static_cast<int>(description.length())); // command length
    outgoing.writeUnsignedByte(cmdId); // command type
    outgoing.writeUnsignedByte(cmdStatus); // status
    outgoing.writeString(description); // description
}

/**
 * \brief Writes a server version information message response on the outgoing tcpip::Storage.
 */
void traci_api::TraCIServer::writeVersion()
{
    debugPrint("Writing version information");

    this->writeStatusResponse(CMD_GETVERSION, STATUS_OK, "");

    tcpip::Storage answerTmp;
    answerTmp.writeInt(API_VERSION);
    answerTmp.writeString("Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion()));

    outgoing.writeUnsignedByte(1 + 1 + static_cast<int>(answerTmp.size()));
    outgoing.writeUnsignedByte(CMD_GETVERSION);
    outgoing.writeStorage(answerTmp);
}

/**
 * \brief Sends the internally stored outgoing TraCI message to the client. 
 * Should only be called by the server itself on waitForCommands()
 */
void traci_api::TraCIServer::sendResponse()
{
    debugPrint("Sending response to TraCI client");
    debugPrint("Outgoing data: " + outgoing.hexDump());

    ssocket.sendExact(outgoing);
}

void traci_api::TraCIServer::writeToOutputWithSize(tcpip::Storage& storage, bool force_extended)
{
    this->writeToStorageWithSize(storage, outgoing, force_extended);
}

void traci_api::TraCIServer::writeToStorageWithSize(tcpip::Storage& src, tcpip::Storage& dest, bool force_extended)
{
    auto size = 1 + src.size();
    if (size > 255 || force_extended)
    {
        dest.writeUnsignedByte(0);
        dest.writeInt(size);
    }
    else
        dest.writeUnsignedByte(size);

    dest.writeStorage(src);
}

void traci_api::TraCIServer::addSubscription(uint8_t sub_type, std::string object_id, int start_time, int end_time, std::vector<uint8_t> variables)
{
    VariableSubscription* sub;

    switch (sub_type)
    {
    case CMD_SUB_VHCVAR:

        debugPrint("Adding VHC subscription.");

        sub = new VehicleVariableSubscription(object_id, start_time, end_time, variables);
        break;

    case CMD_SUB_SIMVAR:

        debugPrint("Adding SIM subscription.");

        sub = new SimulationVariableSubscription(object_id, start_time, end_time, variables);
        break;

    default:
        writeStatusResponse(sub_type, STATUS_NIMPL, "Subscription type not implemented: " + std::to_string(sub_type));
        return;
    }

    std::string errors;
    tcpip::Storage temp;
    uint8_t result = sub->handleSubscription(temp, true, errors); // validate

    if (result == VariableSubscription::STATUS_EXPIRED)
    {
        debugPrint("Expired subscription.");

        writeStatusResponse(sub_type, STATUS_ERROR, "Expired subscription.");
        return;
    }
    else if (result != VariableSubscription::STATUS_OK)
    {
        debugPrint("Error adding subscription.");

        writeStatusResponse(sub_type, STATUS_ERROR, errors);
        return;
    }

    writeStatusResponse(sub_type, STATUS_OK, "");
    writeToOutputWithSize(temp, true);
    subs.push_back(sub);
}

void traci_api::TraCIServer::processSubscriptions(tcpip::Storage& sub_store)
{
    tcpip::Storage temp;
    tcpip::Storage sub_results;
    uint8_t sub_res;
    std::string errors;
    int count = 0;

    for (auto i = subs.begin(); i != subs.end(); ++i)
    {
        sub_res = (*i)->handleSubscription(temp, false, errors);

        if (sub_res == VariableSubscription::STATUS_EXPIRED || sub_res == VehicleVariableSubscription::STATUS_VHCNOTFOUND)
        {
            subs.erase(i);
            delete *i;
        }
        else if (sub_res == VariableSubscription::STATUS_OK)
        {
            writeToStorageWithSize(temp, sub_results, true);
            count++;
        }

        temp.reset();
    }

    sub_store.writeInt(count);
    sub_store.writeStorage(sub_results);
}


/**
 * \brief Executes a shutdown command, destroying the current connections and closing the socket.
 */
void traci_api::TraCIServer::cmdShutDown()
{
    debugPrint("Got shutdown command, acknowledging and shutting down");

    this->writeStatusResponse(CMD_SHUTDOWN, STATUS_OK, "");
    running = false;
}


/**
 * \brief Runs the simulation.
 * \param target_time The target simulation time. If 0, executes exactly one timestep; if less than the current time, does nothing.
 */
void traci_api::TraCIServer::cmdSimStep(int target_time)
{
    tcpip::Storage subs_store;

    if (Simulation::getInstance()->runSimulation(target_time) >= 0)
        this->writeStatusResponse(CMD_SIMSTEP, STATUS_OK, "");

    // handle subscriptions after simstep command
    tcpip::Storage subscriptions;
    this->processSubscriptions(subscriptions);
    outgoing.writeStorage(subscriptions);
}

/**
 * \brief Gets a variable from the simulation.
 * \param simvar ID of the interal simulation variable to fetch.
 */
void traci_api::TraCIServer::cmdGetSimVar(uint8_t simvar)
{
    tcpip::Storage subs_store;

    if (Simulation::getInstance()->packSimulationVariable(simvar, subs_store))
    {
        this->writeStatusResponse(CMD_GETSIMVAR, STATUS_OK, "");
        this->writeToOutputWithSize(subs_store, false);
    }
    else
    {
        this->writeStatusResponse(CMD_GETSIMVAR, STATUS_NIMPL, ""); // TODO: Cover errors as well!
    }
}

void traci_api::TraCIServer::cmdGetVhcVar(tcpip::Storage& input)
{
    tcpip::Storage result;
    try
    {
        VehicleManager::getInstance()->packVehicleVariable(input, result);
    }
    catch (NotImplementedError& e)
    {
        debugPrint("Variable not implemented");
        debugPrint(e.what());


        this->writeStatusResponse(CMD_GETVHCVAR, STATUS_NIMPL, e.what());
    }
    catch (std::exception& e)
    {
        debugPrint("Fatal error???");
        debugPrint(e.what());

        this->writeStatusResponse(CMD_GETVHCVAR, STATUS_ERROR, e.what());
        throw;
    }

    this->writeStatusResponse(CMD_GETVHCVAR, STATUS_OK, "");
    this->writeToOutputWithSize(result, false);
}

void traci_api::TraCIServer::cmdGetNetworkVar(tcpip::Storage& input, uint8_t cmdid)
{
    tcpip::Storage result;
    try
    {
        if (cmdid == CMD_GETLNKVAR)
            Network::getLinkVariable(input, result);
        else if (cmdid == CMD_GETNDEVAR)
            Network::getJunctionVariable(input, result);
        else
            throw std::runtime_error("???");
    }
    catch (NotImplementedError& e)
    {
        debugPrint("Variable not implemented");
        debugPrint(e.what());


        this->writeStatusResponse(cmdid, STATUS_NIMPL, e.what());
    }
    catch (std::exception& e)
    {
        debugPrint("Fatal error???");
        debugPrint(e.what());

        this->writeStatusResponse(cmdid, STATUS_ERROR, e.what());
        throw;
    }

    this->writeStatusResponse(cmdid, STATUS_OK, "");
    this->writeToOutputWithSize(result, false);
}

void traci_api::TraCIServer::cmdSetVhcState(tcpip::Storage& input)
{
    try
    {
        VehicleManager::getInstance()->setVehicleState(input);
    }
    catch (NotImplementedError& e)
    {
        debugPrint("State change not implemented");
        debugPrint(e.what());


        this->writeStatusResponse(CMD_SETVHCSTATE, STATUS_NIMPL, e.what());
    }
    catch (std::exception& e)
    {
        debugPrint("Fatal error???");
        debugPrint(e.what());

        this->writeStatusResponse(CMD_SETVHCSTATE, STATUS_ERROR, e.what());
        throw;
    }

    this->writeStatusResponse(CMD_SETVHCSTATE, STATUS_OK, "");
}

// ReSharper disable once CppMemberFunctionMayBeStatic
// ReSharper disable once CppMemberFunctionMayBeConst
void traci_api::TraCIServer::cmdGetPolygonVar(tcpip::Storage& input)
{
    /* 
     * we currently don't have polygons in Paramics, at least not API-accesible ones,
     * so we'll just report that there are 0 polygons, to maintain compatibility.
     */

    uint8_t var_id = input.readUnsignedByte();
    std::string pol_id = input.readString();
    tcpip::Storage result;

    result.writeUnsignedByte(RES_GETPOLVAR);
    result.writeUnsignedByte(var_id);
    result.writeString(pol_id);

    try {
        switch (var_id)
        {
        case VARLST:
            result.writeUnsignedByte(VTYPE_STRLST);
            result.writeStringList(std::vector<std::string>()); // hard coded, no polygons
            break;
        case VARCNT:
            result.writeUnsignedByte(VTYPE_INT);
            result.writeInt(0); // hard coded -- no polygons
            break;
        default:
            throw std::exception();
        }

        writeStatusResponse(CMD_GETPOLVAR, STATUS_OK, "");
        writeToOutputWithSize(result, false);
    }
    catch ( ... )
    {
        writeStatusResponse(CMD_GETPOLVAR, STATUS_ERROR, "No such polygon (id: " + pol_id + ")");
    }    
}
