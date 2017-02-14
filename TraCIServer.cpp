#include "TraCIServer.h"
#include "programmer.h"
#include <thread>
#include "shawn/storage.h"
using namespace std;



TraCIServer::TraCIServer()
{
	ssocket = new tcpip::Socket(5000);
	outgoing = new tcpip::Storage();
}


TraCIServer::~TraCIServer()
{
	delete(ssocket);
	delete(outgoing);
}

void TraCIServer::run()
{
	std::string version_str = "Paramics TraCI plugin v" + std::string(PLUGIN_VERSION) + " on Paramics v" + std::to_string(qpg_UTL_parentProductVersion());
	this->p_printf(version_str);
	this->p_printf("Awaiting connections on port 5000");
	ssocket->accept();
	this->p_printf("Accepted connection");
	this->waitForCommands();

}

void TraCIServer::close()
{
	ssocket->close();
}

void TraCIServer::waitForCommands()
{
	tcpip::Storage * incoming = new tcpip::Storage();
	
	if (DEBUG)
		this->p_printf("Waiting for incoming commands from TraCI client...");

	while (ssocket->receiveExact(*incoming))
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
		this->cmdSimulationStep();
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

void TraCIServer::cmdSimulationStep()
{
	if(DEBUG)
		this->p_printf("Running one simulation step...");

	qps_GUI_runSimulation();
	this->writeStatusResponse(CMD_SIMSTEP, STATUS_OK, "");

	// write subscription responses...
	outgoing->writeInt(0);
}

