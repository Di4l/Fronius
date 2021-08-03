/*
 * udp.hpp
 *
 *  Created on: 13 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#ifndef UDP_HPP_
#define UDP_HPP_
//-----------------------------------------------------------------------------
#include "udpsock.hpp"
#include "threads.hpp"
//-----------------------------------------------------------------------------

namespace nsNetwork
{
	//-------------------------------------------------------------------------
	class TUdp;
	//-------------------------------------------------------------------------
	typedef void (*fncOnRecv)(TUdp*);
	//-------------------------------------------------------------------------

	class TUdp
	{
	private:
		TUdpSock          m_sock;
		nsThread::TThread m_thread;

		static void m_fThread(nsThread::TThread* thrd, void* usr);

	protected:
	public:
		TUdp();
		virtual ~TUdp();

		inline TUdpSock&          socket() { return m_sock;   }
		inline nsThread::TThread& thread() { return m_thread; }

		fncOnRecv onReceive;
	};
	//-------------------------------------------------------------------------
} /* namespace nsNetwork */
//-----------------------------------------------------------------------------
#endif /* UDP_HPP_ */
//-----------------------------------------------------------------------------
