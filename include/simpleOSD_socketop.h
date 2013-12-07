/*
 * vim:expandtab:shiftwidth=4:tabstop=4:
 *
 * Copyright   (2013)      Contributors
 * Contributor : chitr   chitr.prayatan@gmail.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 * 
 * ---------------------------------------
 */
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef ERROR
#define ERROR -1
#endif


#define COMPONENT_FSAL LOG_CRITICAL
#define LogCrit        BLOCKFRONT_LOG
#define LogDebug        BLOCKFRONT_LOG
#define LogInfo        BLOCKFRONT_LOG



#ifndef COMPONENT_SOCKET
#define COMPONENT_SOCKET COMPONENT_FSAL
#endif
typedef enum SOCKET_STATUS {
SOC_SUCCESS=0,
SOC_TIMEOUT,
SOC_ERROR
}SOCKET_STATUS_t;
#ifndef RESERVED_FD_COUNT 
#define RESERVED_FD_COUNT 3
#endif
/******* EPOLL  data **********************************************************/
#ifndef EPOLL_ARG1
#define EPOLL_ARG1 1
#endif
#ifndef MAX_EPOLL_EVENT
#define MAX_EPOLL_EVENT 1
#endif
#ifndef SEC2MSEC
#define SEC2MSEC 1000 /*convert from sec to msec*/
#endif
#ifndef ERR_RESET
#define ERR_RESET  \
	errno=0;
#endif

