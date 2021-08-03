/*
 * adapters.h
 *
 *  Created on: 24/09/2010
 *      Author: rhermoso
 */
//-----------------------------------------------------------------------------
#ifndef __ADAPTERS_H__
#define __ADAPTERS_H__
//-----------------------------------------------------------------------------
#ifdef WIN32
	#include <winsock2.h>
	#include <ws2tcpip.h>

#else
	#include <sys/types.h>
	#include <sys/socket.h>
	#include <sys/ioctl.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <net/if.h>
	#include <unistd.h>

	#ifdef __sparc__
		#include <sys/sockio.h>
		#ifndef ifr_netmask
			#define ifr_netmask ifr_addr
		#endif
	#endif

	#define WSADATA short
#endif
//-----------------------------------------------------------------------------
#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <bitset>
//-----------------------------------------------------------------------------

namespace nsNetwork
{
	//-------------------------------------------------------------------------
	enum TAdapterKind { akUnknown = 0, akPoint2Point, akLoopback };
	//-------------------------------------------------------------------------
	enum TTraffic { tUnicast = 0, tBroadcast, tMulticast };
	//-------------------------------------------------------------------------

	class TNetTraffic
	{
	private:
		std::bitset<3> m_bsFlags;
	protected:
	public:
		TNetTraffic();
		virtual ~TNetTraffic();

		bool contains(TTraffic tTraf);
		void clear();
		void fill();

		friend TNetTraffic& operator <<(TNetTraffic& sender, TTraffic tTraf);
		friend TNetTraffic& operator >>(TNetTraffic& sender, TTraffic tTraf);
	};
	//-------------------------------------------------------------------------

	struct SAdapter
	{
		std::string  Name;
		std::string  Alias;
		bool		 Up;
		TAdapterKind Kind;
		TNetTraffic  Allowed;
		unsigned int Address;
		unsigned int Netmask;

		SAdapter() { Name = Alias = ""; Up = false; Kind = akUnknown; Address = Netmask = 0; }
	};
	//-------------------------------------------------------------------------

	class TAdapters
	{
	private:
		static WSADATA      m_wsaData;
		static unsigned int m_uiInstances;

		std::vector<SAdapter*> m_ilInterfaces;

		void clear();
	protected:
	public:
		TAdapters();
		virtual ~TAdapters();

		unsigned int createAdapterList();

		inline size_t size() { return m_ilInterfaces.size(); }

		SAdapter* getAdapterByIndex(unsigned int uiIndex);
		SAdapter* getAdapterByAddress(unsigned int uiIp);
		SAdapter* getAdapterByAddress(in_addr& iaAddress);
		SAdapter* getAdapterByAddress(sockaddr_in& saiAddress);
		SAdapter* getAdapterByAddress(sockaddr& saAddress);

		//-- Devuelve el interfaz cuya ip se ajusta a la direccion que nos han pasado
		SAdapter* matchAdapter(unsigned int& uiIp);
		SAdapter* matchAdapter(sockaddr_in& saiAddress);
		SAdapter* matchAdapter(sockaddr& saAddress);
	};
	//-------------------------------------------------------------------------
	TNetTraffic& operator <<(TNetTraffic& sender, TTraffic tTraf);
	TNetTraffic& operator >>(TNetTraffic& sender, TTraffic tTraf);
	//-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif /* TETHINTERFACES_H_ */
