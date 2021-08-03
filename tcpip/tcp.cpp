/*
 * tcp.cpp
 *
 *  Created on: 13 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#include "tcp.hpp"
//-----------------------------------------------------------------------------
using namespace nsNetwork;
//-----------------------------------------------------------------------------

TTcp::TTcp()
{
	onReceive = NULL;
	m_sock.adapters().createAdapterList();
	m_thread.create(m_fThread, this, true);
}
//-----------------------------------------------------------------------------

TTcp::~TTcp()
{
	onReceive = NULL;
}
//-----------------------------------------------------------------------------

void TTcp::m_fThread(nsThread::TThread* thrd, void* usr)
{
	TTcp* tcp = static_cast<TTcp*>(usr);
	static fd_set  read;
	static timeval tm_out = { 1, 0 };

	if(INVALID_SOCKET != tcp->socket().handle())
	{
		FD_ZERO(&read);
		tcp->socket().fillFd(read);

		int ret = select(tcp->socket().handle() + 1, &read, NULL, NULL, &tm_out);
		if(SOCKET_ERROR == ret)
		{
		}
		else if(ret and tcp->socket().isSetFd(read))
		{
			if(tcp->onReceive)
			{
				tcp->onReceive(tcp);
			}
			else
			{
				char   data[1024];
				size_t sz = 1024;
				tcp->socket().recv(data, sz);
			}
		}
	}
}
//-----------------------------------------------------------------------------





//-----------------------------------------------------------------------------
#ifdef DLLBUILD

#include "tcp.h"
//-----------------------------------------------------------------------------
static nsNetwork::TTcp tcp;
//-----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
{
#endif
//-----------------------------------------------------------------------------

int DLLCALL tcp_open(const char* url)
{
	if(!url)
		return -1;

	tcp.onReceive = NULL;
	tcp.socket().openSocket(std::string(url));
	if(tcp.socket().isServer())
		tcp.thread().resume();

	return 1;
}
//-----------------------------------------------------------------------------

int DLLCALL tcp_close(void)
{
	tcp.socket().closeSocket();
	tcp.thread().terminate();
	tcp.thread().resume();
	tcp.thread().waitFor(4000);
	return 1;
}
//-----------------------------------------------------------------------------

int DLLCALL tcp_set_on_receive(tcpEvent evnt)
{
	tcp.onReceive = reinterpret_cast<fncOnRecv>(evnt);
	return 1;
}
//-----------------------------------------------------------------------------

int DLLCALL tcp_read(char* data, int sz)
{
	size_t l = size_t(sz);
	return tcp.socket().recv(data, l);
}
//-----------------------------------------------------------------------------

int DLLCALL tcp_write(const char* data, int sz)
{
	return tcp.socket().send(data, size_t(sz));
}
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* DLLBUILD */
//-----------------------------------------------------------------------------
