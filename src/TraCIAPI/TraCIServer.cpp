#include "TraCIServer.h"
#include "programmer.h"
#include <thread>
#include "storage.h"
#include <shellapi.h>
#include "Constants.h"

bool starts_with(std::string const& in_string,
				 std::string const& prefix)
{
	return prefix.length() <= in_string.length() &&
		std::equal(prefix.begin(), prefix.end(), in_string.begin());
}


traci_api::TraCIServer::TraCIServer()
{
	running = false;

	//try to get port from command line arguments
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::string prefix(CMDARG_PORT);

	port = DEFAULT_PORT; // if it fails, use the default port
	for (int i = 0; i < argc; i++)
	{
		// convert from widestring to normal string
		std::wstring temp(argv[i]);
		std::string str(temp.begin(), temp.end());

		// check if argument prefix matches
		if (starts_with(str, prefix))
		{
			std::string s_port = str.substr(prefix.length(), str.npos);
			try
			{
				port = std::stoi(s_port);
			} catch(...)
			{
				TraCIServer::p_printf("Invalid port identifier - Falling back to default port");
				port = DEFAULT_PORT;
			}
			
		}
	}

	ssocket = new tcpip::Socket(port);
	outgoing = new tcpip::Storage();
	simulation = new traci_api::Simulation();
}


traci_api::TraCIServer::~TraCIServer()
{
	delete(ssocket);
	delete(outgoing);
	delete(simulation);

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
	ssocket->accept();
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
	ssocket->close();
}


/**
 * \brief Waits for incomming commands on the TCP socket.
 */
void traci_api::TraCIServer::waitForCommands()
{
	tcpip::Storage* incoming = new tcpip::Storage(); // the whole incoming message
	tcpip::Storage* cmdStore = new tcpip::Storage(); // individual commands in the message

	if (DEBUG)
		TraCIServer::p_printf("Waiting for incoming commands from TraCI client...");

	/* While the connection is open, receive commands from the client */
	while (running && ssocket->receiveExact(*incoming))
	{
		auto msize = incoming->size();
		if (DEBUG)
			TraCIServer::p_printf("Got message of length " + std::to_string(msize));

		/* Multiple commands may arrive at once in one message, 
		 * divide them into multiple storages for easy handling */
		while (incoming->size() > 0 && incoming->valid_pos())
		{
			

			uint8_t cmdlen = incoming->readUnsignedByte();
			cmdStore->writeUnsignedByte(cmdlen);

			for (uint8_t i = 0; i < cmdlen - 1; i++)
				cmdStore->writeUnsignedByte(incoming->readUnsignedByte());

			this->parseCommand(*cmdStore);
			cmdStore->reset();			
		}

		this->sendResponse();
		incoming->reset();
		outgoing->reset();
	}

	delete(cmdStore);
	delete(incoming);
}

void traci_api::TraCIServer::cmdSimStep(int target_time) const
{
	tcpip::Storage* subs_store = new tcpip::Storage();
		
	if (simulation->runSimulation(target_time, *subs_store) >= 0)
		this->writeStatusResponse(CMD_SIMSTEP, STATUS_OK, "");

	outgoing->writeStorage(*subs_store); // TODO: FIX, add length
	delete(subs_store);
}

void traci_api::TraCIServer::cmdGetSimVar(uint8_t simvar) const
{
	tcpip::Storage* subs_store = new tcpip::Storage();

	if (simulation->getVariable(simvar, *subs_store))
		this->writeStatusResponse(CMD_GETSIMVAR, STATUS_OK, "");

	outgoing->writeUnsignedByte(1 + subs_store->size());
	outgoing->writeStorage(*subs_store);
	delete(subs_store);
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
void traci_api::TraCIServer::writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description) const
{
	outgoing->writeUnsignedByte(1 + 1 + 1 + 4 + static_cast<int>(description.length())); // command length
	outgoing->writeUnsignedByte(cmdId); // command type
	outgoing->writeUnsignedByte(cmdStatus); // status
	outgoing->writeString(description); // description
}

/**
 * \brief Writes a server version information message response on the outgoing tcpip::Storage.
 */
void traci_api::TraCIServer::writeVersion() const
{
	if (DEBUG)
		TraCIServer::p_printf("Writing version information");

	this->writeStatusResponse(CMD_GETVERSION, STATUS_OK, "");

	tcpip::Storage answerTmp;
	answerTmp.writeInt(API_VERSION);
	answerTmp.writeString("Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion()));

	outgoing->writeUnsignedByte(1 + 1 + static_cast<int>(answerTmp.size()));
	outgoing->writeUnsignedByte(CMD_GETVERSION);
	outgoing->writeStorage(answerTmp);
}

/**
 * \brief Sends the internally stored outgoing TraCI message to the client. 
 * Should only be called by the server itself on waitForCommands()
 */
void traci_api::TraCIServer::sendResponse() const
{
	if (DEBUG)
		TraCIServer::p_printf("Sending response to TraCI client");

	ssocket->sendExact(*outgoing);
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
