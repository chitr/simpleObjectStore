#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/fs.h>
#include <dirent.h>
#include <errno.h>
#include <pthread.h>

#include <linux/fs.h>
#include <sys/epoll.h>

#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"
#include "simpleOSD_fetch_helper.h"

#ifndef GET_OBJECT_MODE 
#define GET_OBJECT_MODE 0100666
#endif 

#define MAX_GET_OBJECT_LOCK 256
pthread_mutex_t fetch_locks[MAX_GET_OBJECT_LOCK]={[0 ... MAX_GET_OBJECT_LOCK-1] = PTHREAD_MUTEX_INITIALIZER};

BLK_OBJ_FETCH_ret 
simpleOSD_get_object_to_dc(blk_obj_fetch_arg_t *p_fetch_arg){
	BLK_OBJ_FETCH_ret ret=BLK_OBJ_FETCH_SUCCESS;
	if(!p_fetch_arg){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid fetch arg");
		ret=BLK_OBJ_FETCH_ERROR;
		goto end;
		}
	char object_path[MAXPATHLEN]={0};
	/*FETCH API*/
#ifdef GET_OBJECT_FROM_DP_NODE	
    pthread_mutex_lock(&fetch_locks[p_fetch_arg->object_index%MAX_GET_OBJECT_LOCK]);
    /*TODO: Add Implementation specific api to get data from data-placement node*/
	pthread_mutex_unlock(&fetch_locks[p_fetch_arg->object_index%MAX_GET_OBJECT_LOCK]);
#else
	/*Following creates object in corresponding osd store */
	if(!p_fetch_arg->p_oid_str){
		if(osd_object_to_dc_path(p_fetch_arg->volume_name,p_fetch_arg->oid,object_path)){
			BLOCKFRONT_LOG(LOG_CRITICAL,"unable to get object path");
		    ret=BLK_OBJ_FETCH_ERROR;
		goto end;
			}
		char cmd[MAXPATHLEN]={0};
		pthread_mutex_lock(&fetch_locks[p_fetch_arg->object_index%MAX_GET_OBJECT_LOCK]);
    struct stat buf;
    if(stat(object_path,&buf)){
    int object_fd=open(object_path, O_WRONLY | O_CREAT ,GET_OBJECT_MODE);		
	if((fallocate(object_fd,0,0,p_fetch_arg->object_size))){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Error to allaocte data file index %d ",p_fetch_arg->object_index);
		pthread_mutex_unlock(&fetch_locks[p_fetch_arg->object_index%MAX_GET_OBJECT_LOCK]);
		ret=BLK_OBJ_FETCH_ERROR;
		goto end;
		}
    	}
	pthread_mutex_unlock(&fetch_locks[p_fetch_arg->object_index%MAX_GET_OBJECT_LOCK]);
		}
#endif	
	end:
		return ret;

}



