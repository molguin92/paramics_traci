#pragma once
#include "programmer.h"
#include <mutex>
#include <unordered_map>
#include "Utils.h"
#include "storage.h"

namespace traci_api
{
	class VehicleManager
	{
	public:

		/* vehicle variables */
		static const uint8_t VAR_VHC_LIST = 0x00;
		static const uint8_t VAR_VHC_COUNT = 0x01;
		static const uint8_t VAR_VHC_SPEED = 0x40;
		static const uint8_t VAR_VHC_POS = 0x42;
		static const uint8_t VAR_VHC_POS3D = 0x39;
		static const uint8_t VAR_VHC_ANGLE = 0x43;
		static const uint8_t VAR_VHC_ROAD = 0x50;
		static const uint8_t VAR_VHC_LANE = 0x51;
		static const uint8_t VAR_VHC_LANEIDX = 0x52;
		static const uint8_t VAR_VHC_TYPE = 0x4f;
		static const uint8_t VAR_VHC_LENGTH = 0x44;
		static const uint8_t VAR_VHC_HEIGHT = 0xbc;
		static const uint8_t VAR_VHC_WIDTH = 0x4d;
		static const uint8_t VAR_VHC_SLOPE = 0x36;

		/* not implemented yet*/
		static const uint8_t VAR_VHC_ROUTE = 0x53;
		static const uint8_t VAR_VHC_ROUTEIDX = 0x69;
		static const uint8_t VAR_VHC_EDGES = 0x54;
		static const uint8_t VAR_VHC_COLOR = 0x45;
		static const uint8_t VAR_VHC_LANEPOS = 0x56;
		static const uint8_t VAR_VHC_DIST = 0x84;
		static const uint8_t VAR_VHC_SIGNALST = 0x5b;
		static const uint8_t VAR_VHC_CO2 = 0x60;
		static const uint8_t VAR_VHC_CO = 0x61;
		static const uint8_t VAR_VHC_HC = 0x62;
		static const uint8_t VAR_VHC_PMX = 0x63;
		static const uint8_t VAR_VHC_NOX = 0x64;
		static const uint8_t VAR_VHC_FUELCONS = 0x65;
		static const uint8_t VAR_VHC_NOISE = 0x66;
		static const uint8_t VAR_VHC_ELECCONS = 0x71;
		static const uint8_t VAR_VHC_BESTLANES = 0xb2;
		static const uint8_t VAR_VHC_STOPSTATE = 0xb5;
		static const uint8_t VAR_VHC_VMAX = 0x41;
		static const uint8_t VAR_VHC_ACCEL = 0x46;
		static const uint8_t VAR_VHC_DECEL = 0x47;
		static const uint8_t VAR_VHC_TAU = 0x48;
		static const uint8_t VAR_VHC_SIGMA = 0x5d;
		static const uint8_t VAR_VHC_SPDFACTOR = 0x5e;
		static const uint8_t VAR_VHC_SPEEDDEV = 0x5f;
		static const uint8_t VAR_VHC_VCLASS = 0x49;
		static const uint8_t VAR_VHC_EMSCLASS = 0x4a;
		static const uint8_t VAR_VHC_SHAPE = 0x4b;
		static const uint8_t VAR_VHC_MINGAP = 0x4c;
		static const uint8_t VAR_VHC_WAITTIME = 0x7a;
		static const uint8_t VAR_VHC_NEXTTLS = 0x70;
		static const uint8_t VAR_VHC_SPEEDMODE = 0xb3;
		static const uint8_t VAR_VHC_ALLOWEDSPD = 0xb7;
		static const uint8_t VAR_VHC_LINE = 0xbd;
		static const uint8_t VAR_VHC_PNUMBER = 0x67;
		static const uint8_t VAR_VHC_VIAEDGES = 0xbe;
		static const uint8_t VAR_VHC_NONTRACISPD = 0xb1;
		static const uint8_t VAR_VHC_VALIDROUTE = 0x92;

		static VehicleManager* getInstance();
		static void deleteInstance();

		void reset();

		tcpip::Storage getVehicleVariable(uint8_t varID, std::string s_vid);

		void vehicleDepart(VEHICLE* vehicle);
		void vehicleArrive(VEHICLE* vehicle);

		std::vector<std::string> getDepartedVehicles();
		std::vector<std::string> getArrivedVehicles();
		int getDepartedVehicleCount();
		int getArrivedVehicleCount();

		int currentVehicleCount();
		std::vector<std::string> getVehiclesInSim();

		float getSpeed(int vid);
		void setSpeed(int vid, float speed);

		PositionalData getPosition(int vid);
		DimensionalData getDimensions(int vid);

		std::string getRoadID(int vid);
		std::string getLaneID(int vid);
		int getLaneIndex(int vid);

		std::string getVehicleType(int vid);

		/* prevent alternative instantiation */
		VehicleManager(VehicleManager const&) = delete;
		void operator=(VehicleManager const&) = delete;

	private:

		static VehicleManager* instance;

		VehicleManager() {}
		~VehicleManager() {}

		std::mutex lock_vhc_lists;

		std::unordered_map<int, VEHICLE*> vehicles_in_sim;
		std::vector<VEHICLE*> departed_vehicles;
		std::vector<VEHICLE*> arrived_vehicles;

		VEHICLE* findVehicle(int vid);
	};

	class NoSuchVHCError: public std::runtime_error
	{
	public:
		explicit NoSuchVHCError(const std::string& _Message)
			: runtime_error(_Message)
		{
		}

		explicit NoSuchVHCError(const char* _Message)
			: runtime_error(_Message)
		{
		}
	};
}

