/* _thrlow.c
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
#include "proc.h"

/*

//LINUX
#include <pthread.h>
int pthread_create(pthread_t *thread, pthread_attr_t *attr, void *(* start_routine)(void *), void *arg);
int pthread_mutex_init(pthread_mutex_t *mutex, pthread_mutex_attr_t *mutex_attr);
int pthread_mutex_lock(pthread_mutex_t *mutex);
int pthread_mutex_unlock(pthread_mutex_t *mutex);
int pthread_mutex_destroy(pthread_mutex_t *mutex);

//WINNT
#include <process.h>
unsigned long _beginthread( void( __cdecl *start_address )( void * ), unsigned stack_size, void *arglist );
HANDLE CreateMutex( LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCTSTR lpName ); 
//HANDLE    CreateMutex (NULL, FALSE, NULL);
DWORD WaitForSingleObject(HANDLE hIOMutex, DWORD wait);
BOOL ReleaseMutex(HANDLE);
BOOL CloseHandle(hIOMutex);


*/


MUTEX mutex_init(void) {

#ifdef WIN32
	return((MUTEX)CreateMutex(NULL, FALSE, NULL));
#endif

#ifdef LINUX
	pthread_mutex_t *mutex;

	mutex = malloc(sizeof(pthread_mutex_t));
	if (!mutex) {
		perror("malloc_mutex\n");
		return(NULL);
	}

	if (pthread_mutex_init(mutex, NULL)) {
		perror("pthread_mutex_init\n");
		return(NULL);
	}

	return(mutex);
#endif
}

void mutex_lock(MUTEX mutex)
{
#ifdef WIN32
	WaitForSingleObject(mutex, INFINITE);
#endif

#ifdef LINUX
	pthread_mutex_lock(mutex);
#endif
}

void mutex_unlock(MUTEX mutex)
{
#ifdef WIN32
	ReleaseMutex(mutex);
#endif

#ifdef LINUX
	pthread_mutex_unlock(mutex);
#endif
}

void mutex_destroy(MUTEX mutex)
{
#ifdef WIN32
	CloseHandle(mutex);
#endif

#ifdef LINUX
	pthread_mutex_destroy(mutex);
	free(mutex);
#endif
}




void thread_create(void *(*start_address)(void *), void *arglist) 
{
#ifdef WIN32
	_beginthread((void (*)(void *))start_address, 0, arglist);
#endif

#ifdef LINUX
	pthread_t thread;
	if (pthread_create(&thread, NULL, start_address, arglist) != 0)
		perror("pthread");
#endif
}


void thread_destroy()
{
#ifdef WIN32
	_endthread();
#endif

#ifdef LINUX
	pthread_exit(NULL);
#endif
}



int thread_self()
{
#ifdef WIN32
	return((int)(GetCurrentThreadId() & 0xFFFF));
#endif

#ifdef LINUX
	return((int)(pthread_self() & 0xFFFF));
#endif	
}

