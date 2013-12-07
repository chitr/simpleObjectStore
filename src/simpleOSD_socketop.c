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
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <time.h> 
#include <assert.h>

#include <sys/stat.h>
#include <netinet/tcp.h>
#include <sys/statfs.h>
#include <utime.h>
#include <sys/epoll.h>



#include "simpleOSD_socketop.h"
#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"


SOCKET_STATUS_t 
sock_wait_epoll(int socketfd,int timeout,int *p_errsv){
    SOCKET_STATUS_t ret=SOC_SUCCESS;

    struct epoll_event event_on_socketfd;
    int epoll_fd = ERROR;
    struct epoll_event processableEvents;
    int numEvents = ERROR;
    int timeout_msec=timeout*SEC2MSEC;
    int so_error=ERROR;
    unsigned so_len=sizeof(so_error);

    ERR_RESET
        if (ERROR==(epoll_fd = epoll_create (EPOLL_ARG1))){	  
            LogCrit(COMPONENT_SOCKET,"Could not create the epoll FD list %s",
                    strerror(*p_errsv));
            *p_errsv=errno;
            ret=SOC_ERROR;
            goto end;
        }     

    event_on_socketfd.data.fd = socketfd;
    event_on_socketfd.events = EPOLLOUT | EPOLLIN | EPOLLERR;

    ERR_RESET
        if(ERROR==epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketfd, &event_on_socketfd)){
            *p_errsv=errno;	
            LogCrit(COMPONENT_SOCKET,"Unable to add socket fd to epoll event list:%s");      	   
            ret=SOC_ERROR;
            goto end;
        }
    ERR_RESET
        numEvents = epoll_wait(epoll_fd, &processableEvents, 
                MAX_EPOLL_EVENT,timeout_msec);

    if (numEvents <= 0){
        *p_errsv=errno;	
        LogCrit(COMPONENT_SOCKET,"epoll Error: epoll_wait :%s",strerror(*p_errsv));
        ret = (numEvents)? SOC_ERROR:SOC_TIMEOUT;
        goto end;
    }
    ERR_RESET
        if (getsockopt(socketfd, SOL_SOCKET,SO_ERROR, &so_error, &so_len) < 0){

            ret=SOC_ERROR;
            *p_errsv = errno;
            BLOCKFRONT_LOG(LOG_CRITICAL,"Socket not ready %s",strerror(*p_errsv));
            goto end;

        }
    if (so_error != 0){  
        ret=SOC_ERROR;
        *p_errsv = so_error;
        BLOCKFRONT_LOG(LOG_CRITICAL,"Socket not ready %s",strerror(*p_errsv));
        goto end;
    }
end:    
    if(epoll_fd > RESERVED_FD_COUNT){
        if(close(epoll_fd)){
            int err=errno;
            BLOCKFRONT_LOG(LOG_CRITICAL,"epoll fd %d failed %s",epoll_fd,strerror(errno));
         }
    }
    return ret;
}

/*wait with select implementataion works only with socket fds < 1024 
   Use epoll based wait otherwise */
SOCKET_STATUS_t 
sock_wait(int socketfd,
	long timeout,
	int rd_fdset,
    int wr_fdset,int *p_errsv){

    SOCKET_STATUS_t ret=SOC_SUCCESS;

    struct timeval tv = {0,0};
    fd_set fdset;
    fd_set *rfds, *wfds;
    int n, so_error;
    unsigned so_len;

    FD_ZERO (&fdset);
    FD_SET  (socketfd, &fdset);
    tv.tv_sec = timeout;
    tv.tv_usec = 0;

    if(p_errsv)
        *p_errsv=0;
    BLOCKFRONT_LOG(LOG_CRITICAL,"wait for connection to get established={%ld,%ld}",
            tv.tv_sec, tv.tv_usec);

    if (rd_fdset) rfds = &fdset; else rfds = NULL;
    if (wr_fdset) wfds = &fdset; else wfds = NULL;

    n = select (socketfd+1, rfds, wfds, NULL, &tv);
    if(p_errsv)
        *p_errsv=errno;
    switch (n) {
        case 0:
            BLOCKFRONT_LOG(LOG_CRITICAL,"This wait timed out");			
            ret=SOC_TIMEOUT;
            break;
        case -1:
            BLOCKFRONT_LOG(LOG_CRITICAL,"This is error during wait");
            ret=SOC_ERROR;
            break;
        default:
            /* select tell us that sock is ready, test it*/
            so_len = sizeof(so_error);
            so_error = 0;
            getsockopt (socketfd, SOL_SOCKET, SO_ERROR, &so_error, &so_len);
            if (so_error != 0){  
				ret=SOC_ERROR;
                *p_errsv = so_error;
                BLOCKFRONT_LOG(LOG_CRITICAL,"wait failed");
            }
    }
end:
    return ret;
}



