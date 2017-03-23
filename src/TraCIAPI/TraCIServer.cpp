#include "TraCIServer.h"
#include "programmer.h"
#include <thread>
#include "storage.h"
#include "Constants.h"
#include "VehicleManager.h"

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
}


traci_api::TraCIServer::~TraCIServer()
{
	VehicleManager::deleteInstance();
	if (DEBUG)
		TraCIServer::p_printf("Server succesfully shut down");
}

/**
 * \brief Starts this instance, binding it to a port and awaiting connections. This method is blocking, and as such should be run in a separate thread.
 */
void traci_api::TraCIServer::run()
{
	running = true;
	std::string version_str = "Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion());
	TraCIServer::p_printf(version_str);
	TraCIServer::p_printf("Simulation start time: " + std::to_string(qpg_CFG_simulationTime()));
	TraCIServer::p_printf("Awaiting connections on port " + std::to_string(port));
	ssocket.accept();
	TraCIServer::p_printf("Accepted connection");
	this->waitForCommands();
}

/**
 * \brief Closes the socket, severing all connections.
 */
void traci_api::TraCIServer::close()
{
	if (DEBUG)
		TraCIServer::p_printf("Closing connections");
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

	if (DEBUG)
		TraCIServer::p_printf("Waiting for incoming commands from TraCI client...");

	/* While the connection is open, receive commands from the client */
	while (running && ssocket.receiveExact(incoming))
	{
		auto msize = incoming.size();
		if (DEBUG)
			TraCIServer::p_printf("Got message of length " + std::to_string(msize));

		/* Multiple commands may arrive at once in one message, 
		 * divide them into multiple storages for easy handling */
		while (incoming.size() > 0 && incoming.valid_pos())
		{
			uint8_t cmdlen = incoming.readUnsignedByte();
			cmdStore.writeUnsignedByte(cmdlen);

			for (uint8_t i = 0; i < cmdlen - 1; i++)
				cmdStore.writeUnsignedByte(incoming.readUnsignedByte());

			this->parseCommand(cmdStore);
			cmdStore.reset();
		}

		this->sendResponse();
		incoming.reset();
		outgoing.reset();
	}
}

/**
 * \brief Parses an incoming command according to the TraCI protocol specifications.
 * \param storage A tcpip::Storage object which contains a single TraCI command.
 */
void traci_api::TraCIServer::parseCommand(tcpip::Storage& storage)
{
	if (DEBUG)
		TraCIServer::p_printf("Parsing command");

	uint8_t cmdLen = storage.readUnsignedByte();
	uint8_t cmdId = storage.readUnsignedByte();
	tcpip::Storage state;

	if (DEBUG)
	{
		TraCIServer::p_printf("Command length: " + std::to_string(cmdLen));
		TraCIServer::p_printf("Command ID: " + std::to_string(cmdId));
	}

	switch (cmdId)
	{
	case CMD_GETVERSION:
		if (DEBUG)
			TraCIServer::p_printf("Got CMD_GETVERSION");
		this->writeVersion();
		break;

	case CMD_SIMSTEP:
		if (DEBUG)
			TraCIServer::p_printf("Got CMD_SIMSTEP");

		this->cmdSimStep(storage.readInt());
		break;

	case CMD_SHUTDOWN:
		if (DEBUG)
			TraCIServer::p_printf("Got CMD_SHUTDOWN");
		this->cmdShutDown();
		break;

	case CMD_GETSIMVAR:
		if (DEBUG)
			TraCIServer::p_printf("Got CMD_GETSIMVAR");
		this->cmdGetSimVar(storage.readUnsignedByte());
		break;

	case CMD_SETVHCSTATE:
		if (DEBUG)
			TraCIServer::p_printf("Got CMD_SETVHCSTATE");
		for (int i = 0; i < cmdLen - 2; i++)
			state.writeUnsignedByte(storage.readUnsignedByte());
		this->cmdSetVhcState(state);
		break;

	case CMD_GETVHCVAR:
		if (DEBUG)
			TraCIServer::p_printf("Got CMD_GETVHCVAR");
		{
			tcpip::Storage input;
			while (storage.valid_pos())
				input.writeUnsignedByte(storage.readUnsignedByte());
			this->cmdGetVhcVar(input);
		}
		break;
	default:
		if (DEBUG)
			TraCIServer::p_printf("Command not implemented!");

		writeStatusResponse(cmdId, STATUS_NIMPL, "Method not implemented.");
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
	if (DEBUG)
		TraCIServer::p_printf("Writing version information");

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
	if (DEBUG)
		TraCIServer::p_printf("Sending response to TraCI client");

	ssocket.sendExact(outgoing);
}

void traci_api::TraCIServer::writeToOutputWithSize(tcpip::Storage& storage)
{
	auto size = 1 + storage.size();
	if (size > 255)
	{
		outgoing.writeUnsignedByte(0);
		outgoing.writeInt(size);
	}
	else
		outgoing.writeUnsignedByte(size);

	outgoing.writeStorage(storage);
}


/**
 * \brief Convenience function, prints an std::string on Paramics' output window.
 * \param text 
 */
void traci_api::TraCIServer::p_printf(std::string text)
{
	text = "TraCI: " + text;
	qps_GUI_printf(&text[0u]);
}


/**
 * \brief Executes a shutdown command, destroying the current connections and closing the socket.
 */
void traci_api::TraCIServer::cmdShutDown()
{
	if (DEBUG)
		TraCIServer::p_printf("Got shutdown command, acknowledging and shutting down");

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

	if (Simulation::getInstance()->runSimulation(target_time, subs_store) >= 0)
		this->writeStatusResponse(CMD_SIMSTEP, STATUS_OK, "");

	outgoing.writeStorage(subs_store); // TODO: FIX, add length
}

/**
 * \brief Gets a variable from the simulation.
 * \param simvar ID of the interal simulation variable to fetch.
 */
void traci_api::TraCIServer::cmdGetSimVar(uint8_t simvar)
{
	tcpip::Storage subs_store;

	if (Simulation::getInstance()->getVariable(simvar, subs_store))
	{
		this->writeStatusResponse(CMD_GETSIMVAR, STATUS_OK, "");
		this->writeToOutputWithSize(subs_store);
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
		
		VehicleManager::getInstance()->getVehicleVariable(input, result);
	}
	catch (NotImplementedError& e)
	{
		if(DEBUG)
		{
			TraCIServer::p_printf("Variable not implemented");
			TraCIServer::p_printf(e.what());
		}
			
		this->writeStatusResponse(CMD_GETVHCVAR, STATUS_NIMPL, e.what());
	}
	/*catch (std::runtime_error& e)
	{
		if (DEBUG)
		{
			TraCIServer::p_printf("Runtime error");
			TraCIServer::p_printf(e.what());
		}

		this->writeStatusResponse(CMD_GETVHCVAR, STATUS_ERROR, e.what());
	}
	catch (traci_api::NoSuchVHCError& e)
	{
		if (DEBUG)
		{
			TraCIServer::p_printf("No such vehicle");
			TraCIServer::p_printf(e.what());
		}

		this->writeStatusResponse(CMD_GETVHCVAR, STATUS_ERROR, e.what());
	}*/
	catch (std::exception& e)
	{
		if (DEBUG)
		{
			TraCIServer::p_printf("Fatal error???");
			TraCIServer::p_printf(e.what());
		}
		this->writeStatusResponse(CMD_GETVHCVAR, STATUS_ERROR, e.what());
		throw;
	}

	this->writeStatusResponse(CMD_GETVHCVAR, STATUS_OK, "");
	this->writeToOutputWithSize(result);
}


void traci_api::TraCIServer::cmdSetVhcState(tcpip::Storage& state)
{
	try
	{
		Simulation::getInstance()->setVhcState(state);
		this->writeStatusResponse(CMD_SETVHCSTATE, STATUS_OK, "");
	}
	catch (...)
	{
		this->writeStatusResponse(CMD_SETVHCSTATE, STATUS_NIMPL, ""); // TODO: Cover errors as well!
	}
}
