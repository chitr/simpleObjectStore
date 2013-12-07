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
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <linux/fs.h>
#include <unistd.h>
#include <errno.h>
#include <dirent.h>

#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"


extern osd_lun_t *p_simpleOSD_lun_list[NB_MAX_LUN];
extern blk_io_op_t simpleOSD_blk_op;
#ifndef dprintf
#define dprintf printf
#endif
#ifndef eprintf
#define eprintf printf
#endif
extern osd_helper_func_t osd_op_func;

/*Simple OSD  blk io will be intialised from simpleOSD bs library */
blk_io_op_t simpleOSD_blk_op={NULL,NULL,NULL,NULL,NULL};


BLK_IO_RETURN_t
simpleOSD_blk_read(void *p_read_in,     /*IN*/
        void *p_read_out) { /*OUT*/

    BLK_IO_RETURN_t ret=BLK_IO_SUCCESS;
    blk_io_args_t *p_read_args_in = (blk_io_args_t *)p_read_in;
    blk_io_args_t *p_read_args_out = (blk_io_args_t *)p_read_out;


    
    int lun_index=p_read_args_in->lun_index;
	size_t buf_len=p_read_args_in->buf_len;
	size_t offset=p_read_args_in->offset;

	void *buf=p_read_args_out->buf;
	osd_collection_t *p_dc_collection=NULL;
	osd_lun_t *p_lun=NULL;


    if(!(p_lun=(osd_lun_t*)(VAL_POINTER(p_simpleOSD_lun_list[lun_index])))){
		 BLOCKFRONT_LOG(LOG_CRITICAL,"\n Lun is not initialsed Lun index %d",lun_index);
		ret=BLK_IO_ERROR;
		goto end;
    	}

	if(!(p_dc_collection=p_lun->datacache_info.osd_collection_list)){
		 BLOCKFRONT_LOG(LOG_CRITICAL,"\n Lun is not initialsed Lun index %d",lun_index);
		ret=BLK_IO_ERROR;
		goto end;
		}
	#if SUPPORT_UNFORMATTED_DISK /*Provisioning pre formatted disks*/
	if(FALSE == p_lun->is_fs_done)
		goto end;
	#endif
    if(osd_op_func.pfn_offset_read(p_dc_collection,buf,buf_len,offset)){
		 BLOCKFRONT_LOG(LOG_CRITICAL,"\n Read failed index %d,offset %lu len %lu",lun_index,offset,buf_len);
		ret=BLK_IO_ERROR;
		goto end;		
		}

    BLOCKFRONT_LOG(LOG_DEBUG,"\n eighkpc_pread  osd success %d :%lu \n",lun_index,offset);
end:
    return ret;

}


BLK_IO_RETURN_t
simpleOSD_blk_write(void *p_write_in,     /*IN*/
        void *p_write_out) {   /*OUT*/
    BLK_IO_RETURN_t ret=BLK_IO_SUCCESS;
    blk_io_args_t *p_write_args_in = (blk_io_args_t *)p_write_in;
    blk_io_args_t *p_write_args_out = (blk_io_args_t *)p_write_out;	


    
    int lun_index=p_write_args_in->lun_index;
	size_t buf_len=p_write_args_in->buf_len;
	size_t offset=p_write_args_in->offset;

	void *buf=p_write_args_in->buf;
	osd_collection_t *p_dc_collection=NULL;
	osd_lun_t *p_lun=NULL;


    if(!(p_lun=(osd_lun_t*)(VAL_POINTER(p_simpleOSD_lun_list[lun_index])))){
		 BLOCKFRONT_LOG(LOG_CRITICAL,"\n Lun is not initialsed Lun index %d",lun_index);
		ret=BLK_IO_ERROR;
		goto end;
    	}
	
	#ifdef SUPPORT_UNFORMATTED_DISK /*Provisioning for pre formatted disks*/
    if(p_lun->is_fs_done==FALSE)
		p_lun->is_fs_done=TRUE;
	#endif
	if(!(p_dc_collection=p_lun->datacache_info.osd_collection_list)){
		 BLOCKFRONT_LOG(LOG_CRITICAL,"\n Lun is not initialsed Lun index %d",lun_index);
		ret=BLK_IO_ERROR;
		goto end;
		}

    if(osd_op_func.pfn_offset_write(p_dc_collection,buf,buf_len,offset)){
		 BLOCKFRONT_LOG(LOG_CRITICAL,"\n write failed index %d,offset %lu len %lu",lun_index,offset,buf_len);
		ret=BLK_IO_ERROR;
		goto end;		
		}

    BLOCKFRONT_LOG(LOG_DEBUG," eighkpc_pwrite  osd success %d :%lu \n",lun_index,offset);	
end:
    return ret;
}

BLK_IO_RETURN_t
simpleOSD_blk_datasync (void *p_datasync_in,     /*IN*/
                            void *p_datasync_out) {   /*OUT*/
                      BLK_IO_RETURN_t ret=BLK_IO_SUCCESS;
    blk_io_args_t *p_datasync_args_in = (blk_io_args_t *)p_datasync_in;
	blk_io_args_t *p_datasync_args_out = (blk_io_args_t *)p_datasync_out;					  
end:
	return ret;
}  

BLK_IO_RETURN_t
simpleOSD_blk_unmap_file_region(void *p_unmap_in,     /*IN*/
        void *p_unmap_out) {   /*OUT*/
    BLK_IO_RETURN_t ret=BLK_IO_SUCCESS;
    blk_io_args_t *p_unmap_args_in = (blk_io_args_t *)p_unmap_in;
    blk_io_args_t *p_unmap_args_out = (blk_io_args_t *)p_unmap_out;					  
end:
    return ret;
} 

BLK_IO_RETURN_t
simpleOSD_blk_posix_advise(void *p_padvise_in,             /*IN*/
        void *p_padvise_out) {   /*OUT*/
    BLK_IO_RETURN_t ret=BLK_IO_SUCCESS;
    blk_io_args_t *p_padvise_args_in = (blk_io_args_t *)p_padvise_in;
    blk_io_args_t *p_padvise_args_out = (blk_io_args_t *)p_padvise_out;					  
end:
    return ret;
} 


__attribute__((constructor(PRIORITY_INIT_BLK_FUNCS))) static void 
                              simpleOSD_blk_op_constructor(void) {   	
    simpleOSD_blk_op.pfn_read64=simpleOSD_blk_read;
    simpleOSD_blk_op.pfn_write64=simpleOSD_blk_write;
    simpleOSD_blk_op.pfn_data_sync=simpleOSD_blk_datasync;
    simpleOSD_blk_op.pfn_unmap_file_region=simpleOSD_blk_unmap_file_region;
    simpleOSD_blk_op.pfn_posix_advise=simpleOSD_blk_posix_advise;
}

