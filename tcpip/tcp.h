/*
 * tcp.h
 *
 *  Created on: 13 mar. 2018
 *      Author: rhermoso
 */


//-----------------------------------------------------------------------------
#ifndef TCP_H_
#define TCP_H_
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

typedef void (*tcpEvent)(void*);
//-----------------------------------------------------------------------------

int DLLCALL tcp_open(const char* url);
int DLLCALL tcp_close(void);
int DLLCALL tcp_set_on_receive(tcpEvent evnt);
int DLLCALL tcp_read(char* data, int sz);
int DLLCALL tcp_write(const char* data, int sz);
//-----------------------------------------------------------------------------
#ifdef __cplusplus
}
#endif
//-----------------------------------------------------------------------------
#endif /* TCP_H_ */
//-----------------------------------------------------------------------------
