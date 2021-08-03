/*
 * mb_config.h
 *
 *  Created on: 8 nov. 2019
 *      Author: Administrador
 */

//-----------------------------------------------------------------------------
#ifndef MB_CONFIG_H_
#define MB_CONFIG_H_
//-----------------------------------------------------------------------------
#include <cstring>
#include <ostream>
#include "modbus.hpp"
//-----------------------------------------------------------------------------
#define INVERTER_ID		1
#define METER_ID		240
//-----------------------------------------------------------------------------

union mb_ret
{
	char           byte;
	unsigned char  ubyte;
	short          word;
	unsigned short uword;
	int            dword;
	unsigned int   udword;
	float          real;
};
//-----------------------------------------------------------------------------

struct mb_data
{
	unsigned short address;
	unsigned short quantity;
};
#define READ_HOLDING_REGS(id, arr, i, u)	readHoldingRegisters(id, arr[i].address, arr[i].quantity, &u)
//-----------------------------------------------------------------------------

#define INVADDR_MANUFACTURER	0
#define INVADDR_MODEL		1
#define INVADDR_MODEL_MAP	2
#define INVADDR_OPERATING_STATE	3
#define INVADDR_ECPCONN		4
#define INVADDR_DC_VOLTAGE	5
#define INVADDR_DC_POWER	6
#define INVADDR_DCSF_CURRENT	7
#define INVADDR_DCSF_VOLTAGE	8
#define INVADDR_DCSF_POWER	9
#define INVADDR_DC_CURRENT_S1	10
#define INVADDR_DC_VOLTAGE_S1	11
#define INVADDR_DC_POWER_S1	12
#define INVADDR_DC_STATUS_S1	13
#define INVADDR_DC_POWER_S2	14
#define INVADDR_DC_STATUS_S2	15
#define INVADDR_DC_VOLTAGE_S2	16

static const mb_data mbd_inverter[] = {
	  { 40004, 16 }		//-- Manufacturer
	, { 40020, 20 }		//-- Model
	, { 40069,  1 }		//-- Model Map
	, { 40117,  1 }		//-- Operating State
	, { 40195,  1 }		//-- ECPConn: 1 = feeding to the grid, 0 = not feeding
//	, { 40103,  2 }		//-- DC Current Value
	, { 40105,  2 }		//-- DC Voltage Value
	, { 40107,  2 }		//-- DC Power Value
	, { 40264,  2 }		//-- DC Current Scale Factor
	, { 40265,  2 }		//-- DC Voltage Scale Factor
	, { 40266,  2 }		//-- DC Power Scale Factor
	, { 40281,  2 }		//-- DC Current Value (string 1)
	, { 40282,  2 }		//-- DC Voltage Value (string 1)
	, { 40283,  2 }		//-- DC Power (string 1)
	, { 40289,  2 }		//-- DC Status (string 1)
	, { 40303,  2 }     	//-- DC Power (string 2)
	, { 40309,  2 }     	//-- DC Status (string 2)
	, { 40302,  2 }		//-- DC Voltage (string 2)
};

#define readInverter(m, i, u)		m.READ_HOLDING_REGS(INVERTER_ID, mbd_inverter, i, u)
//-----------------------------------------------------------------------------

#define METADDR_MANUFACTURER	0
#define METADDR_MODEL		1
#define METADDR_MODEL_MAP	2
#define METADDR_AC_CURRENT	3
#define METADDR_AC_POWER	4

static const mb_data mbd_meter[] = {
	  { 40004, 16 }		//-- Manufacturer
	, { 40020, 20 }		//-- Model
	, { 40069,  1 }		//-- Model Map
	, { 40071,  2 }		//-- AC Total Current
	, { 40097,  2 }		//-- AC Total Power
};

#define readMeter(m, i, u)		m.READ_HOLDING_REGS(METER_ID, mbd_meter, i, u)
//-----------------------------------------------------------------------------

enum EOpState { oeUnknown = 0		//-- Unknown (first entry)
			  , oeOff     = 1
			  , oeSleeping			//-- Auto shutdown
			  , oeStarting
			  , oeMPPT				//-- Working Normally
			  , oeThrottled			//-- Power reduction active
			  , oeShuttingDown
			  , oeFault				//-- One or more faults present
			  , oeStandby
			  , oeNoBusInit			//-- No SolarNet communication
			  , oeNoCommInv			//-- No communication with inverter possible
			  , oeSnOvercurrent		//-- Overcurrent detected on SolarNet
			  , oeBootload			//-- Iverter currently being updated
			  , oeAfci				//-- AFCI event (arc detection)
			  , oeLastEntry			//-- Final entry. Not real one
};

static const std::string osString[] = {
	"Unknown State code",
	"OFF",
	"Auto shutdown",
	"Inverter is starting",
	"Working normally",
	"Power reduction active",
	"Inverter is shutting down",
	"One or more faults present, see St* or Evt* register",
	"Standby",
	"No SolarNet communication",
	"No communication with inverter possible",
	"Over-current detected on SolarNet plug",
	"Inverter is currently being updated",
	"AFCI event (arc detection)"
};

inline std::string operatingState(const unsigned short ucCode)
{
	return ucCode < oeLastEntry ? osString[ucCode] : osString[oeUnknown];
}
//-----------------------------------------------------------------------------
#endif /* MB_CONFIG_H_ */
//-----------------------------------------------------------------------------
