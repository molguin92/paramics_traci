#include "Utils.h"
#include "programmer.h"
#include "Constants.h"


/**
* \brief Convenience function, prints an std::string on Paramics' output window.
* \param text
*/
void traci_api::printToParamics(std::string text)
{
	text = "TraCI: " + text;
	qps_GUI_printf(&text[0u]);
}

bool traci_api::readTypeCheckingInt(tcpip::Storage& inputStorage, int& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_INT)
	{
		return false;
	}
	into = inputStorage.readInt();
	return true;
}

bool traci_api::readTypeCheckingDouble(tcpip::Storage& inputStorage, double& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_DOUBLE)
	{
		return false;
	}
	into = inputStorage.readDouble();
	return true;
}

bool traci_api::readTypeCheckingString(tcpip::Storage& inputStorage, std::string& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_STR)
	{
		return false;
	}
	into = inputStorage.readString();
	return true;
}

bool traci_api::readTypeCheckingStringList(tcpip::Storage& inputStorage, std::vector<std::string>& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_STRLST)
	{
		return false;
	}
	into = inputStorage.readStringList();
	return true;
}

bool traci_api::readTypeCheckingPosition2D(tcpip::Storage& inputStorage, Vector2D& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_POSITION)
	{
		return false;
	}
	into.x = inputStorage.readDouble();
	into.y = inputStorage.readDouble();
	return true;
}

bool traci_api::readTypeCheckingByte(tcpip::Storage& inputStorage, int8_t& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_BYTE)
	{
		return false;
	}
	into = inputStorage.readByte();
	return true;
}

bool traci_api::readTypeCheckingUnsignedByte(tcpip::Storage& inputStorage, uint8_t& into)
{
	if (inputStorage.readUnsignedByte() != VTYPE_UBYTE)
	{
		return false;
	}
	into = inputStorage.readUnsignedByte();
	return true;
}
