/* -----------------------------------------------------------------------
 * Paramics Programmer API    (paramics-support@quadstone.com)
 * Quadstone Ltd.             Tel: +44 131 220 4491
 * 16 Chester Street          Fax: +44 131 220 4492
 * Edinburgh, EH3 7RA, UK     WWW: http://www.paramics-online.com
 * ----------------------------------------------------------------------- */

#include "programmer.h"
#include "TraCIServer.h"
#include <thread>


std::thread *runner;
TraCIServer *server;


void runner_fn()
{
	server = new TraCIServer();
	server->run();
	server->close();
	delete(server);
}


// Called once after the network is loaded.
void qpx_NET_postOpen(void)
{
	qps_GUI_singleStep(PTRUE);
	TraCIServer::p_printf("TraCI support enabled");
	runner = new std::thread(runner_fn);
}
