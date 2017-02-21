/* -----------------------------------------------------------------------
 * Paramics Programmer API    (paramics-support@quadstone.com)
 * Quadstone Ltd.             Tel: +44 131 220 4491
 * 16 Chester Street          Fax: +44 131 220 4492
 * Edinburgh, EH3 7RA, UK     WWW: http://www.paramics-online.com
 * ----------------------------------------------------------------------- */

#include "programmer.h"
#include <thread>
#include "TraCIAPI/TraCIServer.h"
#include <shellapi.h>

#define DEFAULT_PORT 5000
#define CMDARG_PORT "--traci_port="


std::thread *runner;
traci_api::TraCIServer *server;

bool starts_with(std::string const& in_string,
	std::string const& prefix)
{
	return prefix.length() <= in_string.length() &&
		std::equal(prefix.begin(), prefix.end(), in_string.begin());
}

void runner_fn()
{
	//try to get port from command line arguments
	int argc;
	LPWSTR* argv = CommandLineToArgvW(GetCommandLineW(), &argc);
	std::string prefix(CMDARG_PORT);

	int port = DEFAULT_PORT; // if it fails, use the default port
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
			}
			catch (...)
			{
				traci_api::TraCIServer::p_printf("Invalid port identifier - Falling back to default port");
				port = DEFAULT_PORT;
			}

		}
	}



	server = new traci_api::TraCIServer(port);
	server->run();
	server->close();
	delete(server);
}


// Called once after the network is loaded.
void qpx_NET_postOpen(void)
{
	qps_GUI_singleStep(PTRUE);
	traci_api::TraCIServer::p_printf("TraCI support enabled");
	runner = new std::thread(runner_fn);
}

void qpx_NET_reload()
{
	server->close();
	runner->join();
	qpx_NET_postOpen();
}

void qpx_VHC_release(VEHICLE* vehicle)
{
	server->simulation->vehicleDepart(vehicle);
}

void qpx_VHC_arrive(VEHICLE* vehicle)
{
	server->simulation->vehicleArrive(vehicle);
}
