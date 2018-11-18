/* proc.h
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

#ifndef _PROC
#define _PROC

#ifdef WIN32
#include <process.h>
#include <windows.h>
#define	MUTEX HANDLE
#endif
#ifdef LINUX
#include <pthread.h>
#define	MUTEX pthread_mutex_t *
#endif



MUTEX mutex_init(void);
void mutex_lock(MUTEX mutex);
void mutex_unlock(MUTEX mutex);
void mutex_destroy(MUTEX mutex);


void thread_create(void *(*start_address)(void *), void *arglist);
void thread_destroy();
int	 thread_self();


#endif

