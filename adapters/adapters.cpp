/*
 * TEthInterfaces.cpp
 *
 *  Created on: 24/09/2010
 *      Author: rhermoso
 */
//-----------------------------------------------------------------------------
#include "adapters.hpp"

#include <iomanip>
#include <cstring>
//-----------------------------------------------------------------------------
#define MAX_IFS	64
//-----------------------------------------------------------------------------
using namespace nsNetwork;
//-----------------------------------------------------------------------------

TNetTraffic::TNetTraffic()
{
	m_bsFlags.reset();
}
//-----------------------------------------------------------------------------

TNetTraffic::~TNetTraffic()
{
	m_bsFlags.reset();
}
//-----------------------------------------------------------------------------

bool TNetTraffic::contains(TTraffic tTraf)
{
	return m_bsFlags[tTraf];
}
//-----------------------------------------------------------------------------

void TNetTraffic::clear()
{
	m_bsFlags.reset();
}
//-----------------------------------------------------------------------------

void TNetTraffic::fill()
{
	m_bsFlags.set();
}
//-----------------------------------------------------------------------------

TNetTraffic& nsNetwork::operator <<(TNetTraffic& sender, TTraffic tTraf)
{
	sender.m_bsFlags.set(tTraf);
	return sender;
}
//-----------------------------------------------------------------------------

TNetTraffic& nsNetwork::operator >>(TNetTraffic& sender, TTraffic tTraf)
{
	sender.m_bsFlags.reset(tTraf);
	return sender;
}
//-----------------------------------------------------------------------------








WSADATA      TAdapters::m_wsaData;
unsigned int TAdapters::m_uiInstances = 0;
//-----------------------------------------------------------------------------

TAdapters::TAdapters()
{
	#ifdef WIN32
	if(!m_uiInstances && WSAStartup(MAKEWORD(2, 2), &m_wsaData) != 0)
		throw;
	#endif
	++m_uiInstances;
}
//-----------------------------------------------------------------------------

TAdapters::~TAdapters()
{
	--m_uiInstances;
	#ifdef WIN32
	if(!m_uiInstances)
		WSACleanup();
	#endif
}
//-----------------------------------------------------------------------------

void TAdapters::clear()
{
	std::vector<SAdapter*>::iterator it = m_ilInterfaces.begin();

	while(it != m_ilInterfaces.end())
	{
		delete *it;
		*it = NULL;
		m_ilInterfaces.erase(it);
		it = m_ilInterfaces.begin();
	}
}
//-----------------------------------------------------------------------------

#ifdef WIN32
unsigned int TAdapters::createAdapterList()
{
	clear();

    SOCKET sd = WSASocket(AF_INET, SOCK_DGRAM, 0, 0, 0, 0);
    if(sd == SOCKET_ERROR)
        return 0;

    INTERFACE_INFO InterfaceList[20];
    unsigned long nBytesReturned;
    if(WSAIoctl(sd, SIO_GET_INTERFACE_LIST, 0, 0, &InterfaceList, 20 * sizeof(INTERFACE_INFO), &nBytesReturned, 0, 0) == SOCKET_ERROR)
		return 0;

    int nNumInterfaces = nBytesReturned / sizeof(INTERFACE_INFO);
    for(int i = 0; i < nNumInterfaces; ++i)
    {
    	SAdapter    *adapter = new SAdapter;
        sockaddr_in *pAddress;

        pAddress = (sockaddr_in*)&(InterfaceList[i].iiAddress);
        adapter->Address = pAddress->sin_addr.s_addr;

        pAddress = (sockaddr_in*)&(InterfaceList[i].iiBroadcastAddress);

        pAddress = (sockaddr_in*)&(InterfaceList[i].iiNetmask);
        adapter->Netmask = pAddress->sin_addr.s_addr;

        u_long nFlags = InterfaceList[i].iiFlags;
        adapter->Up = (nFlags & IFF_UP);

        if(nFlags & IFF_POINTTOPOINT)
        	adapter->Kind  = akPoint2Point;
        if(nFlags & IFF_LOOPBACK)
        	adapter->Kind = akLoopback;

        if(nFlags & IFF_BROADCAST)
        	adapter->Allowed << tBroadcast;
        if(nFlags & IFF_MULTICAST)
        	adapter->Allowed << tMulticast;

        m_ilInterfaces.push_back(adapter);
    }

    return m_ilInterfaces.size();
}

#else

unsigned int TAdapters::createAdapterList()
{
	clear();
/*
	ifaddrs *ia_list = NULL;
	ifaddrs *ia_aux  = NULL;

	if(getifaddrs(&ia_list) < 0)
	{
		std::cout << "Error en llamada a la funcion getifaddrs\n";
		return 0;
	}

	ia_aux = ia_list;
	while(ia_aux != NULL)
	{
		if(ia_aux->ifa_addr->sa_family == AF_INET)
		{
			SAdapter *adapter = new SAdapter;

			adapter->Name       = ia_aux->ifa_name;
			adapter->Address    = ((sockaddr_in*)ia_aux->ifa_addr)->sin_addr.s_addr;
			adapter->SubnetMask = ((sockaddr_in*)ia_aux->ifa_netmask)->sin_addr.s_addr;

			m_ilInterfaces.push_back(adapter);
		}
		ia_aux = ia_aux->ifa_next;
	}
	freeifaddrs(ia_list);

	return (unsigned int)m_ilInterfaces.size();
*/
	ifconf ifc;
    ifreq *ifr;
    int    sock, ifnum;

    memset(&ifc, 0, sizeof(ifconf));
    ifc.ifc_len = 0;
    ifc.ifc_ifcu.ifcu_req = NULL;

    sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP);

	#ifndef __sparc__
    if(ioctl(sock, SIOCGIFCONF, &ifc) < 0)
        return 0;
	#else
		ifc.ifc_len = 10 * sizeof(ifreq);
	#endif

    ifr = (ifreq*)(new char[ifc.ifc_len]);
    ifc.ifc_ifcu.ifcu_req = ifr;
		
    if(ioctl(sock, SIOCGIFCONF, &ifc) < 0)
    {
    	delete[] ifr;
        return 0;
    }

    ifnum = ifc.ifc_len / sizeof(ifreq);
	for(int i = 0; i < ifnum; ++i)
	{
        if(ifr[i].ifr_addr.sa_family == AF_INET)
        {
			SAdapter *adapter = new SAdapter;

			adapter->Name    = ifr[i].ifr_name;
			adapter->Address = ((sockaddr_in*)&ifr[i].ifr_addr)->sin_addr.s_addr;
        	//-- Obtenemos la mascara de subred para esta interfaz
            if(ioctl(sock, SIOCGIFNETMASK, &ifr[i]) >= 0)
            	adapter->Netmask = ((sockaddr_in*)&ifr[i].ifr_netmask)->sin_addr.s_addr;
            if(ioctl(sock, SIOCGIFFLAGS, &ifr[i]) >= 0)
            {
            	adapter->Up = (ifr[i].ifr_flags & IFF_UP);

            	if(ifr[i].ifr_flags & IFF_POINTOPOINT)
                	adapter->Kind  = akPoint2Point;
                if(ifr[i].ifr_flags & IFF_LOOPBACK)
                	adapter->Kind = akLoopback;

                if(ifr[i].ifr_flags & IFF_BROADCAST)
                	adapter->Allowed << tBroadcast;
                if(ifr[i].ifr_flags & IFF_MULTICAST)
                	adapter->Allowed << tMulticast;
            }
			m_ilInterfaces.push_back(adapter);
        }
    }
    close(sock);
	delete[] ifr;

    return 0;

}
#endif
//-----------------------------------------------------------------------------

SAdapter* TAdapters::getAdapterByIndex(unsigned int uiIndex)
{
	return (uiIndex < m_ilInterfaces.size() ? m_ilInterfaces[uiIndex] : NULL);
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::getAdapterByAddress(unsigned int uiIp)
{
	std::vector<SAdapter*>::iterator it = m_ilInterfaces.begin();
	SAdapter* ret = NULL;

	while(it != m_ilInterfaces.end() && ret == NULL)
	{
		if((*it)->Address == uiIp)
			ret = *it;
		++it;
	}

	return ret;
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::getAdapterByAddress(in_addr& iaAddress)
{
	unsigned int ui_addr = iaAddress.s_addr;
	return getAdapterByAddress(ui_addr);
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::getAdapterByAddress(sockaddr_in& saiAddress)
{
	return getAdapterByAddress(saiAddress.sin_addr);
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::getAdapterByAddress(sockaddr& saAddress)
{
	sockaddr_in* sai_addr = (sockaddr_in*)&saAddress;
	return getAdapterByAddress(*sai_addr);
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::matchAdapter(unsigned int& uiIp)
{
	sockaddr_in sai_addr;
	memset(&sai_addr, 0, sizeof(sockaddr_in));
	sai_addr.sin_family = AF_INET;
	sai_addr.sin_addr.s_addr = uiIp;

	return matchAdapter(sai_addr);
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::matchAdapter(sockaddr_in& saiAddress)
{
	unsigned int ui_mask = 0xffffffff;
	unsigned int ui_aux;
	SAdapter*    ret     = NULL;

	if(IN_MULTICAST(ntohl(saiAddress.sin_addr.s_addr)))
		ui_mask = htonl(0x00ffffff);

	std::vector<SAdapter*>::iterator it = m_ilInterfaces.begin();
	while(it != m_ilInterfaces.end() && ret == NULL)
	{
		ui_aux = (ui_mask & (*it)->Netmask);
		if(ui_aux != 0)
		{
			unsigned int ui_addr = saiAddress.sin_addr.s_addr;
			if(((*it)->Address & ui_aux) == (ui_addr & ui_aux))
				ret = *it;
		}
		++it;
	}

	return ret;
}
//-----------------------------------------------------------------------------

SAdapter* TAdapters::matchAdapter(sockaddr& saAddress)
{
	sockaddr_in* sai_addr = (sockaddr_in*)&saAddress;
	return matchAdapter(*sai_addr);
}
//-----------------------------------------------------------------------------
