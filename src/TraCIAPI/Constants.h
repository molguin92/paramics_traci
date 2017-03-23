#pragma once
#ifdef _DEBUG
#define DEBUG true
#else
#define DEBUG false
#endif

//Server definitions
#define API_VERSION 10
#define PLUGIN_VERSION "0.11alpha"


namespace traci_api
{
	//variable types
	static const uint8_t VTYPE_UBYTE = 0x07;
	static const uint8_t VTYPE_BYTE = 0x08;
	static const uint8_t VTYPE_INT = 0x09;
	static const uint8_t VTYPE_FLOAT = 0x0a;
	static const uint8_t VTYPE_DOUBLE = 0x0b;
	static const uint8_t VTYPE_STR = 0x0c;
	static const uint8_t VTYPE_STRLST = 0x0e;
	static const uint8_t VTYPE_COBJ = 0x0f;

	static const uint8_t VTYPE_POSITION = 0x01;
	static const uint8_t VTYPE_POSITION3D = 0x03;
	static const uint8_t VTYPE_ROADMAPPOS = 0x04;
	static const uint8_t VTYPE_LONLAT = 0x00;
	static const uint8_t VTYPE_LONLATALT = 0x02;
	static const uint8_t BOUNDBOX = 0x05;
	static const uint8_t VTYPE_POLYGON = 0x06;
	static const uint8_t VTYPE_TLIGHTPHASELST = 0x0d;
	static const uint8_t VTYPE_COLOR = 0x11;
	

	//Status responses:
	static const uint8_t STATUS_OK = 0x00;
	static const uint8_t STATUS_ERROR = 0x0ff;
	static const uint8_t STATUS_NIMPL = 0x01;

	//Command definitions:
	//Simulation control:
	static const uint8_t CMD_GETVERSION = 0x00;
	static const uint8_t CMD_SIMSTEP = 0x02;
	static const uint8_t CMD_SHUTDOWN = 0x7f;
	//------------------
	//Value retrieval:
	static const uint8_t CMD_GETSIMVAR = 0xab;
	static const uint8_t RES_GETSIMVAR = 0xbb;

	static const uint8_t CMD_GETVHCVAR = 0xa4;
	static const uint8_t RES_GETVHCVAR = 0xb4;
	//Change State:
	static const uint8_t CMD_SETVHCSTATE = 0xc4;


}