/*
 * TUdp.cpp
 *
 *  Created on: 5 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#include "udpsock.hpp"
#include <cstring>
//-----------------------------------------------------------------------------
using namespace nsNetwork;
//-----------------------------------------------------------------------------

void TUdpSock::createSocket()
{
	handle() = socket(AF_INET, SOCK_DGRAM, 0);
}
//-----------------------------------------------------------------------------

int TUdpSock::send(const char* data, size_t sz)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	return sendTo(data, sz, addr);
}
//-----------------------------------------------------------------------------

int TUdpSock::recv(char* data, size_t& sz)
{
	sockaddr_in addr;
	memset(&addr, 0, sizeof(sockaddr_in));
	return recvFrom(data, sz, &addr);
}
//-----------------------------------------------------------------------------
