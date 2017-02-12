/* -----------------------------------------------------------------------
 * Paramics Programmer API    (paramics-support@quadstone.com)
 * Quadstone Ltd.             Tel: +44 131 220 4491
 * 16 Chester Street          Fax: +44 131 220 4492
 * Edinburgh, EH3 7RA, UK     WWW: http://www.paramics-online.com
 * ----------------------------------------------------------------------- */

#include "programmer.h"
#include <thread>

using namespace std;

thread *runner;
BOOL running;


void runner_fn()
{
	while(running)
	{
		this_thread::sleep_for(100ms);
		qps_GUI_printf("pajaro ctm");
		qps_GUI_runSimulation();
	}
}


// Called once after the network is loaded.
void qpx_NET_postOpen(void)
{
	qps_GUI_singleStep(PTRUE);
	running = PTRUE;

	runner = new thread(runner_fn);
}
