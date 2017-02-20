/* -----------------------------------------------------------------------
 * Paramics Programmer API    (paramics-support@quadstone.com)
 * Quadstone Ltd.             Tel: +44 131 220 4491
 * 16 Chester Street          Fax: +44 131 220 4492
 * Edinburgh, EH3 7RA, UK     WWW: http://www.paramics-online.com
 * ----------------------------------------------------------------------- */

#include "programmer.h"
#include <thread>
#include "TraCIAPI/TraCIServer.h"


std::thread *runner;
traci_api::TraCIServer *server;


void runner_fn()
{
	server = new traci_api::TraCIServer();
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
	server->simulation->releaseVehicle(vehicle);
}
