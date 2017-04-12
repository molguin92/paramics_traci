#include "Network.h"
#include "programmer.h"
#include "Constants.h"

void traci_api::Network::getLinkVariable(tcpip::Storage& input, tcpip::Storage& output) throw(traci_api::NoSuchLNKError)
{
    uint8_t varID = input.readUnsignedByte();
    std::string lnkID = input.readString();
    LINK* lnk;

    if (varID != VAR_LNK_LST && varID != VAR_LNK_CNT)
    {
        lnk = qpg_NET_link(&lnkID[0]);
        if (!lnk)
            throw traci_api::NoSuchLNKError("No such edge: " + lnkID);
    }

    output.writeUnsignedByte(RES_GETLNKVAR);
    output.writeUnsignedByte(varID);
    output.writeString(lnkID);

    switch (varID)
    {
    case VAR_LNK_LST:
        output.writeUnsignedByte(VTYPE_STRLST);
        {
            std::vector<std::string> edges;
            int lnks = qpg_NET_links();
            for (int i = 1; i <= lnks; i++)
                edges.push_back(qpg_LNK_name(qpg_NET_linkByIndex(i)));
            output.writeStringList(edges);
        }
        break;
    case VAR_LNK_CNT:
        output.writeUnsignedByte(VTYPE_INT);
        output.writeInt(qpg_NET_links());
        break;
    default:
        throw NotImplementedError("");
    }
}

void traci_api::Network::getJunctionVariable(tcpip::Storage& input, tcpip::Storage& output) throw(traci_api::NoSuchNDEError)
{
    uint8_t varID = input.readUnsignedByte();
    std::string ndeID = input.readString();
    NODE* node;

    if (varID != VAR_NDE_LST && varID != VAR_NDE_CNT)
    {
        node = qpg_NET_node(&ndeID[0]);
        if (!node)
            throw traci_api::NoSuchLNKError("No such node: " + ndeID);
    }

    output.writeUnsignedByte(RES_GETNDEVAR);
    output.writeUnsignedByte(varID);
    output.writeString(ndeID);

    switch (varID)
    {
    case VAR_NDE_LST:
        output.writeUnsignedByte(VTYPE_STRLST);
        {
            std::vector<std::string> nodes;
            int node_count = qpg_NET_nodes();
            for (int i = 1; i <= node_count; i++)
                nodes.push_back(qpg_NDE_name(qpg_NET_nodeByIndex(i)));
            output.writeStringList(nodes);
        }
        break;

    case VAR_NDE_CNT:
        output.writeUnsignedByte(VTYPE_INT);
        output.writeInt(qpg_NET_nodes());
        break;

    case VAR_NDE_POS:
        output.writeUnsignedByte(VTYPE_POSITION);
        {
            float x, y, z;
            // ReSharper disable once CppLocalVariableMightNotBeInitialized
            qpg_POS_node(node, &x, &y, &z);
            output.writeDouble(x);
            output.writeDouble(y);
        }
        break;

    case VAR_NDE_SHP:
    default:
        throw NotImplementedError("");
    }
}
