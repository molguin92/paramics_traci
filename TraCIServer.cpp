#include "TraCIServer.h"
#include "programmer.h"
#include <thread>
#include "shawn/storage.h"
using namespace std;



TraCIServer::TraCIServer()
{
	port = DEFAULT_PORT; //TODO: dynamically assign port.
	ssocket = new tcpip::Socket(port);
	outgoing = new tcpip::Storage();
}


TraCIServer::~TraCIServer()
{
	delete(ssocket);
	delete(outgoing);

	if (DEBUG)
		this->p_printf("Server succesfully shut down");
}

/**
 * \brief Starts this instance, binding it to a port and awaiting connections. This method is blocking, and as such should be run in a separate thread.
 */
void TraCIServer::run()
{
	running = true;
	std::string version_str = "Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion());
	this->p_printf(version_str);
	this->p_printf("Simulation start time: " + std::to_string(qpg_CFG_simulationTime()));
	this->p_printf("Awaiting connections on port " + std::to_string(port));
	ssocket->accept();
	this->p_printf("Accepted connection");
	this->waitForCommands();

}

/**
 * \brief Closes the socket, severing all connections.
 */
void TraCIServer::close()
{
	if (DEBUG)
		this->p_printf("Closing connections");
	ssocket->close();
}


/**
 * \brief Waits for incomming commands on the TCP socket.
 */
void TraCIServer::waitForCommands()
{
	tcpip::Storage * incoming = new tcpip::Storage();
	
	if (DEBUG)
		this->p_printf("Waiting for incoming commands from TraCI client...");

	while (running && ssocket->receiveExact(*incoming))
	{

		auto msize = incoming->size();
		if (DEBUG)
			this->p_printf("Got message of length " + std::to_string(msize));

		while (incoming->size() > 0 && incoming->valid_pos())
		{
			tcpip::Storage *cmdStore = new tcpip::Storage();

			uint8_t cmdlen = incoming->readUnsignedByte();
			cmdStore->writeUnsignedByte(cmdlen);

			for (uint8_t i = 0; i < cmdlen - 1; i++)
				cmdStore->writeUnsignedByte(incoming->readUnsignedByte());

			this->parseCommand(*cmdStore);
			delete(cmdStore);
		}

		this->sendResponse();
		incoming->reset();
		outgoing->reset();
	}

	delete(incoming);
}

void TraCIServer::parseCommand(tcpip::Storage& storage)
{
	if (DEBUG)
		this->p_printf("Parsing command");

	uint8_t cmdLen = storage.readUnsignedByte();
	uint8_t cmdId = storage.readUnsignedByte();

	if (DEBUG)
	{
		this->p_printf("Command length: " + std::to_string(cmdLen));
		this->p_printf("Command ID: " + std::to_string(cmdId));
	}

	switch (cmdId)
	{
	case CMD_GETVERSION:
		if (DEBUG)
			this->p_printf("Got CMD_GETVERSION");
		this->writeVersion();
		break;

	case CMD_SIMSTEP:
		if (DEBUG)
			this->p_printf("Got CMD_SIMSTEP");

		this->cmdSimulationStep(storage.readInt());
		break;

	case CMD_SHUTDOWN:
		if (DEBUG)
			this->p_printf("Got CMD_SHUTDOWN");
		this->cmdShutDown();
		break;

	default:
		if (DEBUG)
			this->p_printf("Command not implemented!");

		writeStatusResponse(cmdId, STATUS_NIMPL, "Method not implemented.");
	}
}

void TraCIServer::writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description)
{
	outgoing->writeUnsignedByte(1 + 1 + 1 + 4 + static_cast<int>(description.length())); // command length
	outgoing->writeUnsignedByte(cmdId); // command type
	outgoing->writeUnsignedByte(cmdStatus); // status
	outgoing->writeString(description); // description
}

void TraCIServer::writeVersion()
{
	if(DEBUG)
		this->p_printf("Writing version information");

	uint8_t id = CMD_GETVERSION;
	uint8_t result = STATUS_OK; 
	uint8_t version = API_VERSION;
	std::string description = "";
	std::string version_str = "Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion());

	this->writeStatusResponse(id, result, description);

	tcpip::Storage answerTmp;
	answerTmp.writeInt(version);
	answerTmp.writeString(version_str);

	outgoing->writeUnsignedByte(1 + 1 + static_cast<int>(answerTmp.size()));
	outgoing->writeUnsignedByte(CMD_GETVERSION);
	outgoing->writeStorage(answerTmp);
}

void TraCIServer::sendResponse()
{
	if (DEBUG)
		this->p_printf("Sending response to TraCI client");

	ssocket->sendExact(*outgoing);
}

void TraCIServer::p_printf(std::string text)
{
	text = "TraCI: " + text;
	qps_GUI_printf(&text[0u]);
}

void TraCIServer::cmdSimulationStep(uint32_t ms)
{
	auto current_simtime = qpg_CFG_simulationTime();
	auto target_simtime = ms / 1000.0;

	if(ms == 0)
	{
		if (DEBUG)
			this->p_printf("Running one simulation step...");

		qps_GUI_runSimulation();
	}
	else if (target_simtime > current_simtime)
	{
		if (DEBUG)
		{
			this->p_printf("Running simulation up to target time: " + std::to_string(target_simtime));
			this->p_printf("Current time: " + std::to_string(current_simtime));
		}

		while (target_simtime > current_simtime)
		{
			qps_GUI_runSimulation();
			current_simtime = qpg_CFG_simulationTime();

			if (DEBUG)
				this->p_printf("Current time: " + std::to_string(current_simtime));
		}
	}
	else
	{
		if (DEBUG)
		{
			this->p_printf("Invalid target simulation time: " + std::to_string(ms));
			this->p_printf("Current simulation time: " + std::to_string(current_simtime));
			this->p_printf("Doing nothing");
		}
	}

	this->writeStatusResponse(CMD_SIMSTEP, STATUS_OK, "");

	// write subscription responses...
	outgoing->writeInt(0);
}

void TraCIServer::cmdShutDown()
{
	if (DEBUG)
		this->p_printf("Got shutdown command, acknowledging and shutting down");

	this->writeStatusResponse(CMD_SHUTDOWN, STATUS_OK, "");
	running = false;
}

