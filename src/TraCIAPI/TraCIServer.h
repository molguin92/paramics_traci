#pragma once

#include "socket.h"
#include "storage.h"
#include "Simulation.h"
#include "Subscriptions.h"

namespace traci_api
{
    class TraCIServer
    {
    public:

        TraCIServer(int port);
        ~TraCIServer();
        void run();
        void close();

    private:

        tcpip::Socket ssocket;
        tcpip::Storage outgoing;
        bool running;
        int port;
        std::vector<VariableSubscription*> subs;

        

        void waitForCommands();
        void cmdSimStep(int target_time);
        void cmdGetSimVar(uint8_t simvar);
        void cmdGetVhcVar(tcpip::Storage& input);
        void cmdGetNetworkVar(tcpip::Storage& input, uint8_t cmdid);
        void cmdSetVhcState(tcpip::Storage& input);

        void parseCommand(tcpip::Storage& storage);
        void writeStatusResponse(uint8_t cmdId, uint8_t cmdStatus, std::string description);
        void writeVersion();
        void sendResponse();

        void writeToOutputWithSize(tcpip::Storage& storage, bool force_extended);
        static void writeToStorageWithSize(tcpip::Storage& src, tcpip::Storage& dest, bool force_extended);

        void addSubscription(uint8_t sub_type, std::string object_id, int start_time, int end_time, std::vector<uint8_t> variables);
        void processSubscriptions(tcpip::Storage& sub_store);

        //commands
        void cmdShutDown();
    };

}
