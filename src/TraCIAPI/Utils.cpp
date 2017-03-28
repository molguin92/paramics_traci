#include "Utils.h"
#include "programmer.h"
#include "Constants.h"

namespace traci_api
{
	/**
	* \brief Convenience function, prints an std::string on Paramics' output window.
	* \param text
	*/
	void printToParamics(std::string text)
	{
		text = "TraCI: " + text;
		qps_GUI_printf(&text[0u]);
	}

	bool
	readTYPE_CheckingInt(tcpip::Storage& inputStorage, int& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_INT)
		{
			return false;
		}
		into = inputStorage.readInt();
		return true;
	}


	bool
	readTYPE_CheckingDouble(tcpip::Storage& inputStorage, double& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_DOUBLE)
		{
			return false;
		}
		into = inputStorage.readDouble();
		return true;
	}


	bool
	readTYPE_CheckingString(tcpip::Storage& inputStorage, std::string& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_STR)
		{
			return false;
		}
		into = inputStorage.readString();
		return true;
	}


	bool
	readTYPE_CheckingStringList(tcpip::Storage& inputStorage, std::vector<std::string>& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_STRLST)
		{
			return false;
		}
		into = inputStorage.readStringList();
		return true;
	}


	bool
	readTYPE_CheckingPosition2D(tcpip::Storage& inputStorage, Vector2D& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_POSITION)
		{
			return false;
		}
		into.x = inputStorage.readDouble();
		into.y = inputStorage.readDouble();
		return true;
	}

	bool
	readTYPE_CheckingByte(tcpip::Storage& inputStorage, int8_t& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_BYTE)
		{
			return false;
		}
		into = inputStorage.readByte();
		return true;
	}


	bool
	readTYPE_CheckingUnsignedByte(tcpip::Storage& inputStorage, uint8_t& into)
	{
		if (inputStorage.readUnsignedByte() != VTYPE_UBYTE)
		{
			return false;
		}
		into = inputStorage.readUnsignedByte();
		return true;
	}
}
