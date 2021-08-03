/*
 * TUdp.hpp
 *
 *  Created on: 5 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#ifndef UDPSOCK_HPP_
#define UDPSOCK_HPP_
//-----------------------------------------------------------------------------
#include "socket.hpp"
//-----------------------------------------------------------------------------

namespace nsNetwork
{
	//-------------------------------------------------------------------------

	class TUdpSock : public TSocket
	{
	private:
	protected:
		virtual void createSocket();

	public:
		TUdpSock() : TSocket() {}
		~TUdpSock() {}

		int send(const char* data, size_t sz);
		int recv(char* data, size_t& sz);
		int sendTo(const char* data, size_t sz, sockaddr_in& addr) { return TSocket::sendTo(data, sz, addr);   }
		int recvFrom(char* data, size_t& sz, sockaddr_in* addr)    { return TSocket::recvFrom(data, sz, addr); }
	};
	//-------------------------------------------------------------------------
} /* namespace nsNetwork */
//-----------------------------------------------------------------------------
#endif /* UDPSOCK_HPP_ */
//-----------------------------------------------------------------------------
