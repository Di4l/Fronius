//-----------------------------------------------------------------------------
#ifndef __MODBUS_HPP__
#define __MODBUS_HPP__
//-----------------------------------------------------------------------------
#include <stdio.h>
#include <cstring>
#include <map>
#include <vector>
#include <fstream>

#include "modbus_defines.hpp"
#include "tcpsock.hpp"
//-----------------------------------------------------------------------------

#ifdef WIN32
	#define USE_WINDOWS

	#include <windows.h>
	#include <winsock2.h>
#else
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <netdb.h>
	#include <arpa/inet.h>
	#include <errno.h>
	//-----------------------------------------------------------------------------

	typedef short WORD;
//	typedef short WSADATA;
//	typedef unsigned int SOCKET;
	#define WSAGetLastError()	errno

	#ifndef INVALID_SOCKET
	#define INVALID_SOCKET  (SOCKET)(~0)
	#endif

	#ifndef SOCKET_ERROR
	#define SOCKET_ERROR	(-1)
	#endif

	#ifndef INADDR_ANY
	#define INADDR_ANY      (unsigned long)0
	#endif

	#ifndef ADDR_ANY
	#define ADDR_ANY        INADDR_ANY
	#endif
#endif
//-----------------------------------------------------------------------------

namespace nsModbus
{
	//-- Modbus Application Header. Common to all Modbus protocol versions
	struct MBAP_Header
	{
	    WORD transactionID;
	    WORD protocolID;
	    WORD length;
	    char unitID;
	    char function;
	};
	//-------------------------------------------------------------------------

	//-- Modbus standard packet structure
	struct MB_Std_Message
	{
		MBAP_Header header;
		char 		message[MB_MAX_STANDARD_DATA_LEN];
	};
	//-------------------------------------------------------------------------

	/*--  The following structures are defined for use with devices that	*/
	/*    respond to the "new" Modbus architecture where message fragments	*/
	/*    are allowed 														*/
	struct MB_Fragment
	{
	    char length;		//-- Byte Length (not counting itself). Maximum 197 bytes
	    char fragmentFlag;	//-- Flag for the fragment
	    WORD classID;
	    WORD instanceID;
	    WORD service;
	};
	//-------------------------------------------------------------------------

	//-- Modbus "new" version protocol messaging.
	struct MB_New_Message
	{
		MBAP_Header header;
		MB_Fragment fragment;
		char 		message[MB_MAX_FRAGMENT_DATA_LEN];
	};
	//-------------------------------------------------------------------------





	class TModbus;
    typedef void (*fOnModbusRead)(TModbus*, MB_Std_Message*, void*);
	//-------------------------------------------------------------------------

    class TModbus
	{
	private:
    	static WORD m_transacion_id;

		nsNetwork::TTcpSock m_tcpip;

//        struct MB_UserData
//        {
//            void*    userData;
//            TModbus* sender;
//        } m_udData;

        int  sendRecv(MB_Std_Message& msMssg, void* user = nullptr);
        void socketReceived(char* Data, int Length, void* user);

	protected:
	public:
		TModbus() : m_tcpip(), onReceive(nullptr) {}
		virtual ~TModbus() {}

		void  open(const char* HostName, short Port = MB_PORT);
		void  close();

	    virtual void readCoils(char UnitID, unsigned short StartAddress, unsigned short Quantity, void* UserData);
	    virtual void readDiscreteInputs(char UnitID, unsigned short StartAddress, unsigned short Quantity, void* UserData);
	    virtual void readHoldingRegisters(char UnitID, unsigned short StartAddress, unsigned short Quantity, void* UserData);
	    virtual void readInputRegisters(char UnitID, unsigned short StartAddress, unsigned short Quantity, void* UserData);
	    virtual void writeSingleCoil(char UnitID, unsigned short Address, bool State, void* UserData);
	    virtual void writeSingleRegister(char UnitID, unsigned short Address, short Value, void* UserData);
	    virtual void writeMultipleCoils(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, bool* Values, void* UserData);
	    virtual void writeMultipleRegisters(char UnitID, unsigned short StartAddress,
                            unsigned short Quantity, short* Values, void* UserData);
	    virtual void readFileRecord(char UnitID, unsigned short GroupCount, unsigned short* FileNums,
                            unsigned short* RecordNums, unsigned short* RecordLengths, void* UserData);
	    virtual void writeFileRecord(char UnitID, unsigned short GroupCount, unsigned short* FileNums,
                            unsigned short* RecordNums, unsigned short* RecordLengths, void* UserData);
		virtual void maskWriteRegister(char UnitID, unsigned short Address, unsigned short AndMask,
                            unsigned short OrMask, void* UserData);
	    virtual void readWriteMultipleRegisters(char UnitID, unsigned short ReadStartAddress,
                            unsigned short ReadQuantity, unsigned short WriteStartAddress,
                            unsigned short WriteQuantity, short* Values, void* UserData);
        void readFIFOQueue(char UnitID, unsigned short Address, void* UserData);

        fOnModbusRead  onReceive;
	};
};
//-----------------------------------------------------------------------------
#endif
