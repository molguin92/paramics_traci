/* -----------------------------------------------------------------------
 * Paramics Programmer API    (paramics-support@quadstone.com)
 * Quadstone Ltd.             Tel: +44 131 220 4491
 * 16 Chester Street          Fax: +44 131 220 4492
 * Edinburgh, EH3 7RA, UK     WWW: http://www.paramics-online.com
 * ----------------------------------------------------------------------- */

#include "programmer.h"
#include "TraCIServer.h"
#include <thread>


using namespace std;

thread *runner;
BOOL running;
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
	running = PTRUE;

	runner = new thread(runner_fn);
}
