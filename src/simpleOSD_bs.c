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

#include <linux/fs.h>
#include <sys/epoll.h>
#include <limits.h>
#include <sys/resource.h>





#include "simpleOSD_bs.h"
#include "simpleOSD_mds_helper.h"
#include "simpleOSD_generic.h"
extern blk_io_op_t simpleOSD_blk_op;
extern blk_frontend_parameters_t global_blk_frontend_parameters;

bs_io_op_t simpleOSD_bs_op={NULL,NULL,NULL,NULL,NULL,NULL,NULL};


osd_lun_t *p_simpleOSD_lun_list[NB_MAX_LUN]={0};

BS_IO_RETURN_t
simpleOSD_get_lun_info(osd_lun_t *p_lun){
    BS_IO_RETURN_t ret = BS_IO_SUCCESS;
    
	/*Get get following information from MDS/conf file 
	   No of objects 
	   start object index
	   volume size
	   Block size*/
	blk_mds_args_t lun_info_in,lun_info_out;   
	memset(&lun_info_in,0,sizeof(blk_mds_args_t));
	memset(&lun_info_out,0,sizeof(blk_mds_args_t));
	
	lun_info_in.p_blk_volname=p_lun->lun_vol;
	lun_info_in.op_type=MDS_GET;
	lun_info_in.mds_datatype=BLK_MDS_VOLNAME;

	lun_info_out.mds_datatype=BLK_ENTRY_VOL_META|BLK_MDS_DP_MAP;

	
    lun_info_out.p_dp_data_map=&(p_lun->datacache_info.osd_collection_list[0].dp_data);
	
    if(mds_get_blk_device_info(&lun_info_in,&lun_info_out)){
		ret=BS_IO_ERROR;
		goto end;		
    	}
	
	p_lun->lun_capacity=lun_info_out.blk_volSize;
	p_lun->block_size=lun_info_out.blk_blocksize;
	p_lun->is_fs_done=lun_info_out.is_fs_flag;
	
	osd_collection_t *p_osd_collection=
		               p_lun->datacache_info.osd_collection_list;
	p_osd_collection->nb_objects=lun_info_out.blk_nb_objects;
	p_osd_collection->object_size=lun_info_out.blk_objectsize;
	
	memcpy(p_osd_collection->start_object,lun_info_out.start_oid,MAXOIDLEN);
	snprintf(p_osd_collection->osd_collection_name,MAXNAMESIZE,"%s",
		                                                    p_lun->lun_vol);

	if(osd_init_collection(p_lun)) {
		ret=BS_IO_ERROR;
        goto end;
		}	
end:
    return ret;
}

BS_IO_RETURN_t 
simpleOSD_lun_open(char *path,           /*IN*/
                       int oflag,            /*IN*/
                       uint64_t *size,       /*OUT*/
                       uint32_t *blksize,    /*OUT*/
                       int *lun_index){      /*OUT*/
    BS_IO_RETURN_t ret = BS_IO_SUCCESS;

    int next_free_lun=0;
    int is_free=FALSE;

    for(next_free_lun=0;next_free_lun<NB_MAX_LUN && 
            (p_simpleOSD_lun_list[next_free_lun]);next_free_lun++){		
        if((IS_FREE(p_simpleOSD_lun_list[next_free_lun])))
            break;
    }
    if(next_free_lun>=NB_MAX_LUN){
        ret=BS_IO_ERROR;
        BLOCKFRONT_LOG(LOG_CRITICAL,"\n Reached max LUN limit:%d",NB_MAX_LUN);
        goto end;
    }
    if(!(p_simpleOSD_lun_list[next_free_lun]))	
        p_simpleOSD_lun_list[next_free_lun]=GET_MEMORY(sizeof(osd_lun_t));
    USE_LUN(p_simpleOSD_lun_list[next_free_lun]);

    osd_lun_t *p_curr_lun=(osd_lun_t*)VAL_POINTER(p_simpleOSD_lun_list[next_free_lun]);
    /*Need to modify path name as <dc_path>/dev_blockdev_name and separate 
      block_dev_name from absolute path*/
    
    strncpy(p_curr_lun->lun_vol,path,MAXNAMESIZE);
    p_curr_lun->lun_index=next_free_lun;
    p_curr_lun->io_flag=oflag;

    
	snprintf(p_curr_lun->datacache_info.lun_dc_path,MAXPATHSIZE,"/%s/%s/",
				global_blk_frontend_parameters.dc_path,p_curr_lun->lun_vol);
	
    /*Update Lun from MDS or conf file based on device name */
    if(ret=simpleOSD_get_lun_info(p_curr_lun)){
        ret=BS_IO_ERROR;
        goto end;
    }
	
    *size=p_curr_lun->lun_capacity;
    *blksize=p_curr_lun->block_size;
    *lun_index=next_free_lun;
end:
    return ret;
}

BS_IO_RETURN_t 
simpleOSD_lun_close(int lun_index){ /*IN*/
    BS_IO_RETURN_t ret = BS_IO_SUCCESS;
    if(ret=osd_free_collection( 
       (osd_lun_t*)VAL_POINTER(p_simpleOSD_lun_list[lun_index]))){
        ret = BS_IO_ERROR;
        goto end;
    }
    FREE_LUN(p_simpleOSD_lun_list[lun_index]);
end:	
    return ret;
}
BS_IO_RETURN_t 
simpleOSD_read(int lun_fd,void *buf ,size_t buf_len,size_t offset){
	BS_IO_RETURN_t ret=BS_IO_SUCCESS;
	blk_io_args_t in_args;
	blk_io_args_t out_args;
	memset(&in_args,0,sizeof(blk_io_args_t));
	memset(&out_args,0,sizeof(blk_io_args_t));
	
	in_args.lun_index=lun_fd;
    in_args.buf_len=buf_len;
	in_args.offset= offset;

	out_args.buf=buf;

	if(simpleOSD_blk_op.pfn_read64((void*)&in_args,(void*)&out_args)){
		ret=BS_IO_ERROR;
		goto end;
		}	
end:
	return ret;

}
BS_IO_RETURN_t 
simpleOSD_write(int lun_fd,void *buf ,size_t buf_len,size_t offset){
		BS_IO_RETURN_t ret=BS_IO_SUCCESS;
		
		blk_io_args_t in_args;
		blk_io_args_t out_args;

	in_args.lun_index=lun_fd;
    in_args.buf_len=buf_len;
	in_args.offset= offset;	
	in_args.buf=buf;
	if(simpleOSD_blk_op.pfn_write64((void*)&in_args,(void*)&out_args)){
				ret=BS_IO_ERROR;
		goto end;
		}
	end:
		return ret;

}
BS_IO_RETURN_t 
simpleOSD_datasync(int lun_fd){
		
		BS_IO_RETURN_t ret=BS_IO_SUCCESS;
		
		blk_io_args_t in_args;
		blk_io_args_t out_args;

	in_args.lun_index=lun_fd;

	if(simpleOSD_blk_op.pfn_data_sync((void*)&in_args,(void*)&out_args)){
				ret=BS_IO_ERROR;
		goto end;
		}		
	end:
		return ret;
}


BLK_IO_RETURN_t
simpleOSD_unmap_file_region(int lun_fd,off_t offset,off_t len) {   
	BS_IO_RETURN_t ret=BS_IO_SUCCESS;
	/*TODO*/
end:
	return ret;		  
} 

BLK_IO_RETURN_t
simpleOSD_posix_advise(int fd, off_t offset, off_t len, int advice){   
	BS_IO_RETURN_t ret=BS_IO_SUCCESS;
	/*TODO*/
end:
	return ret;	  
} 


__attribute__((constructor(PRIORITY_INIT_BS_FUNCS))) static void 
                              simpleOSD_blk_op_constructor(void) {   	
    simpleOSD_bs_op.pfn_read64=simpleOSD_read;
    simpleOSD_bs_op.pfn_write64=simpleOSD_write;
    simpleOSD_bs_op.pfn_data_sync=simpleOSD_datasync;
    simpleOSD_bs_op.pfn_unmap_file_region=simpleOSD_unmap_file_region;
    simpleOSD_bs_op.pfn_posix_advise=simpleOSD_posix_advise;
	simpleOSD_bs_op.pfn_open=simpleOSD_lun_open;
	simpleOSD_bs_op.pfn_close=simpleOSD_lun_close;
	
	if(FALSE==global_blk_frontend_parameters.is_intiliazed)
		get_dc_parameters_from_conf(&global_blk_frontend_parameters);	
	struct rlimit ulimit_data;
    struct rlimit ulimit_fd;
    ulimit_fd.rlim_cur = 65535;
    ulimit_fd.rlim_max = 65535;
    ulimit_data.rlim_cur = 9999999998;
    ulimit_data.rlim_max = 9999999998;
    setrlimit(RLIMIT_CORE, &ulimit_data);
    setrlimit(RLIMIT_NOFILE,&ulimit_fd);
}



