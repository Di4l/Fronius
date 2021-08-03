/*
 * tcpsock.cpp
 *
 *  Created on: 5 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#include "tcpsock.hpp"
#include <cstring>
//-----------------------------------------------------------------------------
using namespace nsNetwork;
//-----------------------------------------------------------------------------

void TTcpSock::createSocket()
{
	int i_on = 1;

	handle() = socket(AF_INET, SOCK_STREAM, 0);
    if(setsockopt(handle(), SOL_SOCKET, SO_REUSEADDR, (const char*)&i_on, sizeof(i_on)) != 0)
        closeSocket();
}
//-----------------------------------------------------------------------------

void TTcpSock::openSocket(std::string Url)
{
	TSocket::openSocket(Url);
    if(!connect())
        closeSocket();
}
//-----------------------------------------------------------------------------
