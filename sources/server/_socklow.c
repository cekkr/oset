/* _socklow.c
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

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdarg.h>
#include "sock.h"

void	errexit(const char *format, ...);
SOCKET passiveTCP(unsigned short port,int qlen);
void logfile(char *s, ...);



BOOL server_abort(void)
{
#ifdef WIN32
	return(kbhit());
#endif
#ifdef LINUX
	return(FALSE);
#endif
}

int  sock_error(void)
{
#ifdef WIN32
	return(WSAGetLastError());
#endif
#ifdef LINUX
	return(errno);
#endif
}

void sock_end(void)
{			    
	logfile("SOCK_END\n");
#ifdef WIN32
	WSACleanup();
#endif
#ifdef LINUX
	thread_destroy();
#endif
}

void sock_start(void)
{
#ifdef WIN32
	WSADATA wsaData;
	if (WSAStartup(0x101,&wsaData) == SOCKET_ERROR) 
		errexit("WSAStartup failed with error %d\n",sock_error());
#endif
#ifdef LINUX
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTERM, (void (*)(int))sock_end);
#endif
}


static void *sock_server_thread(void *arg)
{
	struct sock_data *data = (struct sock_data *)arg;

	data->do_comm(data);
	closesocket(data->msgsock);

	logsockfile(data, "Closing connection\n");

	free(data);

	thread_destroy();
	return(NULL);
}

int sock_server(unsigned short port, int qlen, void (*do_comm)(struct sock_data *t))
{
	int fromlen;
	SOCKET listen_socket, msgsock;
	struct sockaddr_in from;
	fd_set	afds,rfds;
	struct timeval timeval = {1,0};
	struct sock_data *sock_data;
	MUTEX mutex;

	mutex = mutex_init();

	listen_socket = passiveTCP(port,qlen);
	printf("Listening on port %d, protocol %s\n",port,"TCP");

	FD_ZERO(&afds);
	FD_SET(listen_socket, &afds);

	while (!server_abort()) {
		memmove(&rfds, &afds, sizeof(rfds));
		if (select(0, &rfds, NULL, NULL, &timeval) < 0) 
			errexit("select() error %d\n", sock_error());

		if (FD_ISSET(listen_socket, &rfds)) {
			/* accept connection */
			fromlen = sizeof(from);
			msgsock = accept(listen_socket,(struct sockaddr*)&from, &fromlen);
			if (msgsock == INVALID_SOCKET) 
				errexit("accept() error %d\n",sock_error());

			sock_data = malloc(sizeof(struct sock_data));
			if (!sock_data)
				errexit("malloc(sock_data) error\n");
			memset(sock_data, 0, sizeof(struct sock_data));
			
			sock_data->mutex = mutex;
			sock_data->msgsock = msgsock;
			sock_data->do_comm = do_comm;
			sock_data->sin_port = from.sin_port;
			memmove(&sock_data->sin_addr, &from.sin_addr, sizeof(struct in_addr));

			thread_create(sock_server_thread, (void *)sock_data);
		}
	}

	closesocket(listen_socket);
	mutex_destroy(mutex);

	sock_end();

	return(0);	
}





SOCKET passiveTCP(unsigned short port, int qlen)
{
	/* return passive socket, aborts on error */
	SOCKET listen_socket;
	struct sockaddr_in local;

	sock_start();

	/* create socket */
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		errexit("Client: Error Opening socket: Error %d\n",sock_error());

	/* bind socket */
	memset(&local, 0, sizeof(local));
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = INADDR_ANY; 
	if (bind(listen_socket,(struct sockaddr*)&local,sizeof(local)) == SOCKET_ERROR) 
		errexit("bind() failed with error %d\n",sock_error());

	/* listen on socket */
	if (listen(listen_socket,qlen) == SOCKET_ERROR) 
		errexit("listen() failed with error %d\n",sock_error());

	return(listen_socket);
}




int sockprintf(struct sock_data *t, char *format, ...)
{
	char buff[300];
	int	ret;
	int	cc;
	char *c;
	va_list args;

	va_start(args, format);
	ret = vsprintf(buff, format, args);
	va_end(args);

	for (c = buff; *c; ++c)
		if (*c == '\n') {
			memmove(c+1,c,strlen(c)+1);
			*c = '\r';
			++c;
			++ret;
		}
	
	if ((cc = send(t->msgsock, buff, ret, 0)) == SOCKET_ERROR) 
		errexit("send error %d", cc);                  

    return(ret);
}          


int sockprintraw(struct sock_data *t, char *buff)
{
	int	cc;
	
	if ((cc = send(t->msgsock, buff, strlen(buff), 0)) == SOCKET_ERROR) 
		errexit("send error %d", cc);                  

    return(0);
}          


char *	sockgets(struct sock_data *t, char *buff, int buffsize)
{
	int cc;
	char *aux;
	int retsize;

	buff[0] = 0;

	while (!t->input_len || (!strstr(t->input_buff, "\r\n") && !strstr(t->input_buff, "\n") && (t->input_len < INPUT_SIZE))) {
		cc = recv(t->msgsock, t->input_buff + t->input_len, INPUT_SIZE - t->input_len, 0);
		if (cc != SOCKET_ERROR) {
			t->input_len += cc;
			t->input_buff[t->input_len] = 0;			
		} else
			break;
	}

	if ((aux = (char *)strstr(t->input_buff, "\r\n")) != NULL) 
		retsize = 2;
	else if ((aux = (char *)strstr(t->input_buff, "\r")) != NULL) 
		retsize = 1;
	else if ((aux = (char *)strstr(t->input_buff, "\n")) != NULL)
		retsize = 1;

	if (aux) {
		int size = min(aux-t->input_buff+retsize, buffsize-1);
		strncpy(buff, t->input_buff, size);
		buff[size] = 0;

		memmove(t->input_buff, t->input_buff+size, INPUT_SIZE+1-size);

		t->input_len -= size;
	}

	return(buff);
}

void	errexit(const char *format, ...)
{
	char buff[300];
	va_list args;

	va_start(args, format);
	vsprintf(buff, format, args);
	va_end(args);
	
	//bitacora(buff);
	//MessageBox(GetActiveWindow(), buff, "", MB_OK|MB_ICONSTOP);
	logfile("%s\n", buff);

	//sock_end();
	thread_destroy();
}




void logsockfile(struct sock_data *sock, char *s, ...)
{
	FILE *f;
	time_t t = time(NULL);
	struct tm *tm = localtime(&t);
	va_list args;
	char buff[200];

	va_start(args, s);

	sprintf(buff, "(%04x)[%s:%d] %s", thread_self(), inet_ntoa(sock->sin_addr), htons(sock->sin_port), s);
	buff[sizeof(buff)-1] = 0;

	vprintf(buff, args);

	if ((f = fopen("oset.log", "at")) != NULL) {
		fprintf(f, "%02i/%02i/%02i %02i:%02i ", tm->tm_mon+1, tm->tm_mday, tm->tm_year % 100, tm->tm_hour, tm->tm_min);
		vfprintf(f, buff, args);
		fclose(f);
	}

	va_end(args);
}
