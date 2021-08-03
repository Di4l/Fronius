/*
 * tcpsock.hpp
 *
 *  Created on: 5 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#ifndef TCPSOCK_HPP_
#define TCPSOCK_HPP_
//-----------------------------------------------------------------------------
#include "socket.hpp"
//-----------------------------------------------------------------------------

namespace nsNetwork
{
	//-------------------------------------------------------------------------

	class TTcpSock : public TSocket
	{
	private:
	protected:
		virtual void createSocket();

	public:
		TTcpSock() : TSocket() {}
		virtual ~TTcpSock() {}

		void openSocket(std::string Url);

		int send(const char* data, size_t sz) { return TSocket::send(data, sz); }
		int recv(char* data, size_t& sz)      { return TSocket::recv(data, sz); }
	};
	//-------------------------------------------------------------------------
} /* namespace nsNetwork */
//-----------------------------------------------------------------------------
#endif /* TCPSOCK_HPP_ */
//-----------------------------------------------------------------------------
