/*
 * udp.cpp
 *
 *  Created on: 13 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#include "udp.hpp"
//-----------------------------------------------------------------------------
using namespace nsNetwork;
//-----------------------------------------------------------------------------

TUdp::TUdp()
{
	onReceive = NULL;
	m_sock.adapters().createAdapterList();
	m_thread.create(m_fThread, this, true);
}
//-----------------------------------------------------------------------------

TUdp::~TUdp()
{
	onReceive = NULL;
}
//-----------------------------------------------------------------------------

void TUdp::m_fThread(nsThread::TThread* thrd, void* usr)
{
	TUdp* udp = static_cast<TUdp*>(usr);
	static fd_set  read;
	static timeval tm_out = { 1, 0 };

	if(INVALID_SOCKET != udp->socket().handle())
	{
		FD_ZERO(&read);
		udp->socket().fillFd(read);

		int ret = select(udp->socket().handle() + 1, &read, NULL, NULL, &tm_out);
		if(SOCKET_ERROR == ret)
		{
		}
		else if(ret and udp->socket().isSetFd(read))
		{
			if(udp->onReceive)
			{
				udp->onReceive(udp);
			}
			else
			{
				char   data[1024];
				size_t sz = 1024;
				udp->socket().recv(data, sz);
			}
		}
	}
}
//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
#ifdef DLLBUILD

#include "udp.h"
//-----------------------------------------------------------------------------
static nsNetwork::TUdp udp;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif
//-----------------------------------------------------------------------------

int DLLCALL udp_open(const char* url)
{
	if(!url)
		return -1;

	udp.onReceive = NULL;
	udp.socket().openSocket(std::string(url));
	if(udp.socket().isServer())
		udp.thread().resume();

	return 1;
}
//-----------------------------------------------------------------------------

int DLLCALL udp_close(void)
{
	udp.socket().closeSocket();
	udp.thread().terminate();
	udp.thread().resume();
	udp.thread().waitFor(4000);
	return 1;
}
//-----------------------------------------------------------------------------

int DLLCALL udp_set_on_receive(udpEvent evnt)
{
	udp.onReceive = reinterpret_cast<fncOnRecv>(evnt);
	return 1;
}
//-----------------------------------------------------------------------------

int DLLCALL udp_read(char* data, int sz)
{
	size_t l = size_t(sz);
	return udp.socket().recv(data, l);
}
//-----------------------------------------------------------------------------

int DLLCALL udp_write(const char* data, int sz)
{
	return udp.socket().send(data, size_t(sz));
}
//-----------------------------------------------------------------------------

int DLLCALL udp_write_to(const char* data, int sz, udp_addr* to)
{
	if(!to)
		return udp_write(data, sz);

	sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_port        = htons(to->port);
	addr.sin_addr.s_addr = htonl(to->addr);
	return udp.socket().sendTo(data, sz, addr);
}
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* DLLBUILD */
//-----------------------------------------------------------------------------
