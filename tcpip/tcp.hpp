/*
 * tcp.hpp
 *
 *  Created on: 13 mar. 2018
 *      Author: rhermoso
 */

//-----------------------------------------------------------------------------
#ifndef TCP_HPP_
#define TCP_HPP_
//-----------------------------------------------------------------------------
#include "tcpsock.hpp"
#include "threads.hpp"
//-----------------------------------------------------------------------------

namespace nsNetwork
{
	//-------------------------------------------------------------------------
	class TTcp;
	//-------------------------------------------------------------------------
	typedef void (*fncOnRecv)(TTcp*);
	//-------------------------------------------------------------------------

	class TTcp
	{
	private:
		TTcpSock          m_sock;
		nsThread::TThread m_thread;

		static void m_fThread(nsThread::TThread* thrd, void* usr);

	protected:
	public:
		TTcp();
		virtual ~TTcp();

		inline TTcpSock&          socket() { return m_sock;   }
		inline nsThread::TThread& thread() { return m_thread; }

		fncOnRecv onReceive;
	};
	//-------------------------------------------------------------------------
} /* namespace nsNetwork */
//-----------------------------------------------------------------------------
#endif /* TCP_HPP_ */
//-----------------------------------------------------------------------------
