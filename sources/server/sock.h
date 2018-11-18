/* sock.h
 *
 * Copyright (C) 2000 Ivan Tubert and Eduardo Tubert
 * 
 * Contact: tubert@eros.pquim.unam.mx
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * All we ask is that proper credit is given for our work, which includes
 * - but is not limited to - adding the above copyright notice to the beginning
 * of your source code files, and to any copyright notice that you may distribute
 * with programs based on this work.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA. 
 * 
 */

#ifndef _SOCK
#define _SOCK

#include "proc.h"

#ifdef WIN32
#undef	BOOL
#include <winsock.h>
#include <conio.h>
#include <process.h>
#endif

#ifdef LINUX
#define	SOCKET  int
#define	BOOL	int
#define	TRUE	1
#define FALSE	0

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <netinet/in.h>
#include <signal.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
extern int errno;

#define closesocket close
#define strnicmp strncasecmp

#ifndef min
#define min(a,b) ((a<b) ? a : b)
#endif

#endif


#define INPUT_SIZE 256

struct sock_data {
	SOCKET msgsock;
	void (*do_comm)(struct sock_data *t);
    u_short sin_port;
    struct  in_addr sin_addr;
	char input_buff[INPUT_SIZE+1];
	int  input_len;
	MUTEX mutex;
};


int sock_server(unsigned short port, int qlen, void (*do_comm)(struct sock_data *t));
int sockprintf(struct sock_data *t, char *format, ...);
int sockprintraw(struct sock_data *t, char *buff);
char *sockgets(struct sock_data *t, char *buff, int buffsize);

void logsockfile(struct sock_data *sock, char *s, ...);


#endif

