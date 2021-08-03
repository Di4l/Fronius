/*
 * udp.h
 *
 *  Created on: 13 mar. 2018
 *      Author: rhermoso
 */


//-----------------------------------------------------------------------------
#ifndef UDP_H_
#define UDP_H_
//-----------------------------------------------------------------------------

#ifdef DLLBUILD
#define DLLCALL		__declspec(dllexport)
#else
#define DLLCALL		__declspec(dllimport)
#endif
//-----------------------------------------------------------------------------

#ifdef __cplusplus
extern "C"
{
#endif
//-----------------------------------------------------------------------------

typedef void (*udpEvent)(void*);
//-----------------------------------------------------------------------------
typedef struct _udp_addr
{
	unsigned int   addr;
	unsigned short port;
} udp_addr;
//-----------------------------------------------------------------------------

int DLLCALL udp_open(const char* url);
int DLLCALL udp_close(void);
int DLLCALL udp_set_on_receive(udpEvent evnt);
int DLLCALL udp_read(char* data, int sz);
int DLLCALL udp_write(const char* data, int sz);
int DLLCALL udp_write_to(const char* data, int sz, udp_addr* to);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* UDP_H_ */
//-----------------------------------------------------------------------------
