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
#include <errno.h>
#define _GNU_SOURCE

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

#include <linux/fs.h>
#include <sys/epoll.h>




#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"

obj_op_func_t simpleOSD_obj_op={NULL,NULL};
OBJ_RETURN_t
simpleOSD_obj_read(object_attr_t *p_object,void *buf,uint64_t local_buf_len,
                                                              uint64_t offset){
	OBJ_RETURN_t ret=OBJ_SUCCESS;	
	uint64_t local_buf_read=0;
	uint64_t buf_read=0;
	int errsv=0;
	while(local_buf_len){
	if(-1==(local_buf_read= 
		pread64(p_object->file_fd,(char *)buf+buf_read,local_buf_len,offset))){
		errsv=errno;
		BLOCKFRONT_LOG(LOG_CRITICAL,"\n Error for reading object   %d :%s,%d,offset %lu:buf len %lu",
					p_object->object_index,strerror(errsv),errsv,offset,local_buf_len);
			ret=OBJ_ERROR;
		/*Should free the file_fd resource*/
		if(osd_reset_cache_object(p_object)){
			BLOCKFRONT_LOG(LOG_CRITICAL,"\n Unable to reset object	%d ",
					p_object->object_index);
			ret=OBJ_ERROR;
			goto end;	
			}
		goto end;
		}			
	buf_read+=local_buf_read;
	local_buf_len-=local_buf_read;
	offset+=local_buf_read;
		}	
end:
	return ret;

}
OBJ_RETURN_t
simpleOSD_obj_write(object_attr_t *p_object,void *buf,uint64_t local_buf_len,
                                                              uint64_t offset){
		OBJ_RETURN_t ret=OBJ_SUCCESS;
	uint64_t local_buf_write=0;
	uint64_t buf_write=0;
	int errsv=0;
	while(local_buf_len){
	if(-1==(local_buf_write= 
		pwrite64(p_object->file_fd,(char *)buf+buf_write,local_buf_len,offset))){
		errsv=errno;
		BLOCKFRONT_LOG(LOG_CRITICAL,"\n Error for write object   %d :%s,%d,offset %lu:buf len %lu",
					p_object->object_index,strerror(errsv),errsv,offset,local_buf_len);
			ret=OBJ_ERROR;
		/*Should free the file_fd resource*/
		if(osd_reset_cache_object(p_object)){
			BLOCKFRONT_LOG(LOG_CRITICAL,"\n Unable to reset object	%d ",
					p_object->object_index);
			ret=OBJ_ERROR;
			goto end;	
			}
		goto end;
		}			
	buf_write+=local_buf_write;
	local_buf_len-=local_buf_write;
	offset+=local_buf_write;
		}			
	end:
		return ret;

}

__attribute__((constructor(PRIORITY_INIT_OBJ_FUNCS))) static void 
                              simpleOSD_obj_op_constructor(void) {   	
    simpleOSD_obj_op.pfn_offset_read=simpleOSD_obj_read;
    simpleOSD_obj_op.pfn_offset_write=simpleOSD_obj_write;
}

