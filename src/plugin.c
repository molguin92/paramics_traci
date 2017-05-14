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
#include "TraCIAPI/VehicleManager.h"
#include "TraCIAPI/Utils.h"

#define DEFAULT_PORT 5000
#define CMDARG_PORT "--traci_port="

std::thread* runner;
traci_api::TraCIServer* server;

/* checks a string for a matching prefix */
bool starts_with(std::string const& in_string,
                 std::string const& prefix)
{
    return prefix.length() <= in_string.length() &&
        std::equal(prefix.begin(), prefix.end(), in_string.begin());
}

void runner_fn()
{
    try {
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
                    traci_api::infoPrint("Invalid port identifier - Falling back to default port");
                    port = DEFAULT_PORT;
                }
            }
        }

        server = new traci_api::TraCIServer(port);
        server->run();
    }
    catch (std::exception& e)
    {
        traci_api::debugPrint("Uncaught while initializing server.");
        traci_api::debugPrint(e.what());
        traci_api::debugPrint("Exiting...");
        throw;
    }
}

// Called once after the network is loaded.
void qpx_NET_postOpen(void)
{
    qps_GUI_singleStep(PFALSE);
    traci_api::infoPrint("TraCI support enabled");
    runner = new std::thread(runner_fn);
}

void qpx_CLK_startOfSimLoop(void)
{
    server->preStep();
}

void qpx_CLK_endOfSimLoop(void)
{
    server->postStep();
}

void close()
{
    runner->join();
    server->close();
    delete server;
    delete runner;
}

void qpx_NET_complete(void)
{
    close();
}

void qpx_NET_close()
{
    close();
}

void qpx_VHC_release(VEHICLE* vehicle)
{
    traci_api::VehicleManager::getInstance()->vehicleDepart(vehicle);
}

void qpx_VHC_arrive(VEHICLE* vehicle, LINK* link, ZONE* zone)

{
    traci_api::VehicleManager::getInstance()->vehicleArrive(vehicle);
}

// routing through TraCI
Bool qpo_RTM_enable(void)
{
    return PTRUE;
}

int qpo_RTM_decision(LINK *linkp, VEHICLE *Vp)
{
    return traci_api::VehicleManager::getInstance()->rerouteVehicle(Vp, linkp);
}

void qpx_VHC_transfer(VEHICLE* vehicle, LINK* link1, LINK* link2)
{
    qps_VHC_destination(vehicle, 0, 0);
}