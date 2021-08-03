/*
 * fsx.cpp
 *
 *  Created on: 10/10/2013
 *      Author: Administrador
 */
//-----------------------------------------------------------------------------
#include "socket.hpp"
#include <sstream>
//-----------------------------------------------------------------------------
using namespace nsNetwork;
//-----------------------------------------------------------------------------
TAdapters TSocket::m_aAdapters;
//-----------------------------------------------------------------------------

TSocket::TSocket() : m_sSocket(INVALID_SOCKET)
{
	m_sSocket = INVALID_SOCKET;
	m_bServer = false;
	m_uiIp    = 0;
	m_uiLan   = 0;
	m_sPort   = 0;
//	createSocket();
}
//-----------------------------------------------------------------------------

TSocket::~TSocket()
{
	closeSocket();
}
//-----------------------------------------------------------------------------

void TSocket::setAdapter(std::string& url)
{
	SAdapter* adapter = m_aAdapters.matchAdapter(m_uiIp);

	m_uiLan = INADDR_ANY;
	if(url.size())
	{
		//-- Se ha incluido a mano la dirección IP de la tarjeta a la que
		//   asociarse
		m_uiLan = inet_addr(url.c_str());
		//-- Confirmamos que la IP suminstrada coincide con la IP de alguna
		//   interfaz en la máquina
		if(!m_aAdapters.getAdapterByAddress(m_uiLan))
		{
			if(adapter)
			{
				//-- De la lista de interfaces, hemos encontrado una que le
				//   vale a la IP suministrada
				m_uiLan = adapter->Address;
			}
			else
			{
				// TODO: throw Error. Could not find any network interface to associate
				// this IP to
			}
		}
	}
	else if(adapter)
	{
		//-- De la lista de interfaces, hemos encontrado una que le
		//   vale a la IP suministrada
		m_uiLan  = adapter->Address;
	}
	else
	{
		// TODO: throw Error. Could not find any network interface to associate
		// this IP to
	}
}
//-----------------------------------------------------------------------------

void TSocket::bindSocket()
{
	sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_port        = m_sPort;
	addr.sin_addr.s_addr = m_uiLan;
	if(SOCKET_ERROR == bind(m_sSocket, (sockaddr*)&addr, sizeof(addr)))
		std::cout << "bind() error: " << m_uiLan << ":" << m_sPort << std::endl;
//	else if(SOCKET_ERROR == connect(m_sSocket, (sockaddr*)&addr, sizeof(addr)))
//		std::cout << "connect() error: " << m_uiLan << ":" << m_sPort << std::endl;
}
//-----------------------------------------------------------------------------

bool TSocket::connect()
{
	sockaddr_in addr;
	addr.sin_family      = AF_INET;
	addr.sin_port        = m_sPort;
	addr.sin_addr.s_addr = m_uiIp;
	return (SOCKET_ERROR == ::connect(m_sSocket, (sockaddr*)&addr, sizeof(addr))) ? false : true;
}
//-----------------------------------------------------------------------------

void TSocket::deleteSocket()
{
	if(INVALID_SOCKET != m_sSocket)
	{
//		shutdown(m_sSocket, SD_RECEIVE);
		::closesocket(m_sSocket);
	}
	m_sSocket = INVALID_SOCKET;
}
//-----------------------------------------------------------------------------

void TSocket::setFromUrl(std::string& Url)
{
	short             s_port = 0;
	std::string       url    = Url;
	std::string       aux;
	std::stringstream ss;
	size_t            pos;

	//-- Presupone un socket cliente
	m_bServer = false;
	//-- Se obtiene el tipo de comunicacion (udp, tcp).
	pos = url.find("://");
	//-- El protocolo se ha comprobado previamente en TIfaceTcp::getTypeFromUrl()

	//-- Se obtiene Puerto de recepcion (para un servidor TCP)
	url = url.substr(pos + 3);
	pos = url.find(":");
	aux = url.substr(0, pos);
	ss << aux;
	ss >> m_sPort;
	ss.clear();
	m_sPort = htons(m_sPort);

	//-- Se obtiene la Direccion IP
	url    = url.substr(pos + 1);
	pos    = url.find(":");
	aux    = url.substr(0, pos);
	m_uiIp = inet_addr(aux.c_str());

	//-- Se obtiene el Puerto de envio (para un cliente TCP)
	url = url.substr(pos + 1);
	pos = url.find("@");
	aux = pos == std::string::npos ? url : url.substr(0, pos);
	ss << aux;
	ss >> s_port;
	ss.clear();
	s_port = htons(s_port);

	//-- Se obtiene la IP de Lan
	url = pos == std::string::npos ? std::string("") : url.substr(pos + 1);
	setAdapter(url);

	//-- Si este socket es cliente, asigna el puerto, si es servidor asocia
	//   a un interfaz
	if(!m_sPort)
		m_sPort  = s_port;
	else
		m_bServer = true;
}
//-----------------------------------------------------------------------------

void TSocket::openSocket(std::string Url)
{
	//-- Se asignan los datos (puerto, ip, lan) a partir de la URL proporcionada
	setFromUrl(Url);
	//-- Se crea el socket
	createSocket();
	if(m_bServer)
		bindSocket();
}
//-----------------------------------------------------------------------------

void TSocket::closeSocket()
{
	deleteSocket();
}
//-----------------------------------------------------------------------------

int TSocket::send(const char* data, size_t sz)
{
	if(NULL == data)
		return 0;

	if(INVALID_SOCKET != m_sSocket)
		return ::send(m_sSocket, (const char*)data, sz, 0);
	return 0;
}
//-----------------------------------------------------------------------------

int TSocket::sendTo(const char* data, size_t sz, sockaddr_in& addr)
{
	if(NULL == data)
		return 0;

	if(INVALID_SOCKET != m_sSocket)
	{
		addr.sin_family = AF_INET;
		if(!addr.sin_port)
			addr.sin_port = m_sPort;
		if(!addr.sin_addr.s_addr)
			addr.sin_addr.s_addr = m_uiIp;
		return ::sendto(m_sSocket, (const char*)data, sz, 0, (sockaddr*)&addr, sizeof(sockaddr_in));
	}
	return 0;
}
//-----------------------------------------------------------------------------

int TSocket::recv(char* data, size_t& sz)
{
	if(!data or !sz)
		return 0;

	if(INVALID_SOCKET != m_sSocket)
		return ::recv(m_sSocket, data, sz, 0);
	//-- todo: ret puede valer SOCKET_ERROR (devuelto por recv()). Tratar?
	return 0;
}
//-----------------------------------------------------------------------------

int TSocket::recvFrom(char* data, size_t& sz, sockaddr_in* addr)
{
	if(!data or !sz)
		return 0;
	if(!addr)
		return recv(data, sz);

	int ret = 0;
	if(INVALID_SOCKET != m_sSocket)
	{
		size_t len = sizeof(sockaddr_in);
		addr->sin_family      = AF_INET;
		addr->sin_port        = 0;
		addr->sin_addr.s_addr = 0;
		ret = ::recvfrom(m_sSocket, data, sz, 0, (sockaddr*)addr, &len);
	}
	//-- todo: ret puede valer SOCKET_ERROR (devuelto por revfrom()). Tratar?
	return ret;
}
//-----------------------------------------------------------------------------

int TSocket::fillFd(fd_set &ReadSet)
{
	if(INVALID_SOCKET != m_sSocket)
	{
		FD_SET(m_sSocket, &ReadSet);
		return m_sSocket;
    }
	return 0;
}
//-----------------------------------------------------------------------------

int TSocket::isSetFd(fd_set &ReadSet)
{
	if(INVALID_SOCKET != m_sSocket)
		return FD_ISSET(m_sSocket, &ReadSet);
	return 0;
}
//-----------------------------------------------------------------------------
