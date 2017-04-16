#include "Simulation.h"
#include <programmer.h>
#include "TraCIServer.h"
#include "Constants.h"
#include "VehicleManager.h"
#include <algorithm>

/*
 * This class abstracts a Paramics traffic simulation, providing helper methods for
 * interacting with the simulator itself.
 */

traci_api::Simulation* traci_api::Simulation::instance = nullptr;

traci_api::Simulation::Simulation(): stepcnt(0)
{
}

traci_api::Simulation::~Simulation()
{
}

float traci_api::Simulation::getCurrentTimeSeconds()
{
    return qpg_CFG_simulationTime();
}

int traci_api::Simulation::getCurrentTimeMilliseconds()
{
    return static_cast<int>(this->getCurrentTimeSeconds() * 1000);
}

void traci_api::Simulation::getRealNetworkBounds(double& llx, double& lly, double& urx, double& ury)
{
    /*
     * Paramics qpg_POS_network() function, which should return the network bounds, does not make sense.
     * It returns coordinates which leave basically the whole network outside of its own bounds. 
     * 
     * Thus, we'll have to "bruteforce" the positional data for the network bounds.
     */

    // get all relevant elements in the network, and all their coordinates

    std::vector<float> x;
    std::vector<float> y;

    int node_count = qpg_NET_nodes();
    int link_count = qpg_NET_links();
    int zone_count = qpg_NET_zones();

    float tempX, tempY, tempZ;

    for(int i = 1; i <= node_count; i++)
    {
        NODE* node = qpg_NET_nodeByIndex(i);
        qpg_POS_node(node, &tempX, &tempY, &tempZ);

        x.push_back(tempX);
        y.push_back(tempY);
    }

    for (int i = 1; i <= zone_count; i++)
    {
        ZONE* zone = qpg_NET_zone(i);
        int vertices = qpg_ZNE_vertices(zone);
        for (int j = 1; j <= vertices; j++)
        {
            qpg_POS_zoneVertex(zone, j, &tempX, &tempY, &tempZ);

            x.push_back(tempX);
            y.push_back(tempY);
        }
    }

    for (int i = 1; i <= link_count; i++)
    {
        // links are always connected to zones or nodes, so we only need
        // to get position data from those that are curved

        LINK* lnk = qpg_NET_linkByIndex(i);
        if(!qpg_LNK_arc(lnk) && !qpg_LNK_arcLeft(lnk))
            continue;

        // arc are perfect sections of circles, thus we only need the start, end and middle point (for all lanes)
        float len = qpg_LNK_length(lnk);
        int lanes = qpg_LNK_lanes(lnk);

        float g, b;

        for(int j = 1; j <= lanes; j++)
        {
            // start points
            qpg_POS_link(lnk, j, 0, &tempX, &tempY, &tempZ, &b, &g);

            x.push_back(tempX);
            y.push_back(tempY);

            // middle points
            qpg_POS_link(lnk, j, len / 2.0, &tempX, &tempY, &tempZ, &b, &g);

            x.push_back(tempX);
            y.push_back(tempY);

            // end points
            qpg_POS_link(lnk, j, len, &tempX, &tempY, &tempZ, &b, &g);

            x.push_back(tempX);
            y.push_back(tempY);
        }

    }
    

    // we have all the coordinates, now get maximums and minimums
    // add some wiggle room as well, just in case
    urx = *std::max_element(x.begin(), x.end()) + 100;
    llx = *std::min_element(x.begin(), x.end()) - 100;
    ury = *std::max_element(y.begin(), y.end()) + 100;
    lly = *std::min_element(y.begin(), y.end()) - 100;
}

traci_api::Simulation* traci_api::Simulation::getInstance()
{
    if (instance == nullptr)
        instance = new Simulation();

    return instance;
}

void traci_api::Simulation::deleteInstance()
{
    if (instance != nullptr)
        delete(instance);

    instance = nullptr;
}

int traci_api::Simulation::runSimulation(uint32_t target_timems)
{
    auto current_simtime = this->getCurrentTimeSeconds();
    auto target_simtime = target_timems / 1000.0;
    int steps_performed = 0;

    traci_api::VehicleManager::getInstance()->reset();

    if (target_timems == 0)
    {
        if (DEBUG)
            printToParamics("Running one simulation step...");

        qps_GUI_runSimulation();
        traci_api::VehicleManager::getInstance()->handleDelayedTriggers();
        steps_performed = 1;
    }
    else if (target_simtime > current_simtime)
    {
        if (DEBUG)
        {
            printToParamics("Running simulation up to target time: " + std::to_string(target_simtime));
            printToParamics("Current time: " + std::to_string(current_simtime));
        }

        while (target_simtime > current_simtime)
        {
            qps_GUI_runSimulation();
            steps_performed++;
            traci_api::VehicleManager::getInstance()->handleDelayedTriggers();

            current_simtime = this->getCurrentTimeSeconds();

            if (DEBUG)
                printToParamics("Current time: " + std::to_string(current_simtime));
        }
    }
    else
    {
        if (DEBUG)
        {
            printToParamics("Invalid target simulation time: " + std::to_string(target_timems));
            printToParamics("Current simulation time: " + std::to_string(current_simtime));
            printToParamics("Doing nothing");
        }
    }

    stepcnt += steps_performed;
    return steps_performed;
}

bool traci_api::Simulation::getVariable(uint8_t varID, tcpip::Storage& result_store)
{
    result_store.writeUnsignedByte(RES_GETSIMVAR);
    result_store.writeUnsignedByte(varID);
    result_store.writeString("");

    VehicleManager* vhcman = traci_api::VehicleManager::getInstance();

    switch (varID)
    {
    case GET_SIMTIME:
        result_store.writeUnsignedByte(VTYPE_INT);
        result_store.writeInt(this->getCurrentTimeMilliseconds());
        break;
    case GET_DEPARTEDVHC_CNT:
        result_store.writeUnsignedByte(VTYPE_INT);
        result_store.writeInt(vhcman->getDepartedVehicleCount());
        break;
    case GET_DEPARTEDVHC_LST:
        result_store.writeUnsignedByte(VTYPE_STRLST);
        result_store.writeStringList(vhcman->getDepartedVehicles());
        break;
    case GET_ARRIVEDVHC_CNT:
        result_store.writeUnsignedByte(VTYPE_INT);
        result_store.writeInt(vhcman->getArrivedVehicleCount());
        break;
    case GET_ARRIVEDVHC_LST:
        result_store.writeUnsignedByte(VTYPE_STRLST);
        result_store.writeStringList(vhcman->getArrivedVehicles());
        break;
    case GET_TIMESTEPSZ:
        result_store.writeUnsignedByte(VTYPE_INT);
        result_store.writeInt(static_cast<int>(qpg_CFG_timeStep() * 1000.0f));
        break;
    case GET_NETWORKBNDS:
        result_store.writeUnsignedByte(VTYPE_BOUNDBOX);
        {
            double llx, lly, urx, ury;
            this->getRealNetworkBounds(llx, lly, urx, ury);

            result_store.writeDouble(llx);
            result_store.writeDouble(lly);
            result_store.writeDouble(urx);
            result_store.writeDouble(ury);
        }
        break;
    default:
        return false;
    }

    return true;
}
