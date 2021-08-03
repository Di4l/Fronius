/*
 * libfsx.hpp
 *
 *  Created on: 10/10/2013
 *      Author: Administrador
 */
//-----------------------------------------------------------------------------
#ifndef __SOCKET_HPP__
#define __SOCKET_HPP__
//-----------------------------------------------------------------------------
#include <adapters.hpp>
//-----------------------------------------------------------------------------
#ifdef WIN32
//	#define close		closesocket
	#define socklent_t	int

	#define SHUT_RD		SD_RECEIVE
	#define SHUT_WR		SD_SEND
	#define SHUT_RDWR	SD_BOTH

#else
//	#include <netdb.h>

	#define closesocket	close
	#define ioctlsocket	ioctl

	#define SD_RECEIVE	SHUT_RD
	#define SD_SEND		SHUT_WR
	#define SD_BOTH		SHUT_RDWR

	typedef short WORD;
	typedef int   SOCKET;

	#define WSAGetLastError()	errno

	#ifndef INVALID_SOCKET
		#define INVALID_SOCKET	(SOCKET)(~0)
	#endif

	#ifndef SOCKET_ERROR
		#define SOCKET_ERROR	(-1)
	#endif

	#ifndef INADDR_ANY
		#define INADDR_ANY		(unsigned long)0
	#endif

	#ifndef ADDR_ANY
		#define ADDR_ANY		INADDR_ANY
	#endif
#endif
//-----------------------------------------------------------------------------

namespace nsNetwork
{
	//-------------------------------------------------------------------------

	class TSocket
	{
	private:
		static TAdapters m_aAdapters;

		SOCKET         m_sSocket;
		bool           m_bServer;
		unsigned int   m_uiLan;
		unsigned int   m_uiIp;
		unsigned short m_sPort;

		void setAdapter(std::string& url);

	protected:
		virtual void createSocket() = 0;
		virtual void bindSocket();
		virtual bool connect();
		virtual void deleteSocket();
		virtual void setFromUrl(std::string& Url);

		virtual int send(const char* data, size_t sz);
		virtual int sendTo(const char* data, size_t sz, sockaddr_in& addr);
		virtual int recv(char* data, size_t& sz);
		virtual int recvFrom(char* data, size_t& sz, sockaddr_in* addr);

	public:
		TSocket();
		virtual ~TSocket();

		inline TAdapters&     adapters() { return m_aAdapters;    }
		inline SOCKET&        handle()   { return m_sSocket;      }
		inline bool           isServer() { return m_bServer;      }
		inline unsigned int   lan()      { return ntohl(m_uiLan); }
		inline unsigned int   ip()       { return ntohl(m_uiIp);  }
		inline unsigned short port()     { return ntohs(m_sPort); }

		virtual void openSocket(std::string Url);
		virtual void closeSocket();

		int fillFd(fd_set &ReadSet);
		int isSetFd(fd_set &ReadSet);
	};
	//-------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------
#endif /* __SOCKET_HPP__ */
//-----------------------------------------------------------------------------
