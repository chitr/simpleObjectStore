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
#include <errno.h>

#include <linux/fs.h>
#include <sys/epoll.h>




#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"
#include "simpleOSD_fetch_helper.h"
extern obj_op_func_t simpleOSD_obj_op;
extern blk_frontend_parameters_t global_blk_frontend_parameters;


osd_helper_func_t osd_op_func={NULL,NULL,NULL};

extern OSD_RETURN_t 
	osd_object_to_dc_path(osd_collection_t *p_dc_collection,
	                           object_attr_t *p_object,char *p_path);

OSD_RETURN_t 
osd_reset_cache_object(object_attr_t *p_object){
    OSD_RETURN_t ret = OSD_SUCCESS;
    int errsv=0;
    if(p_object->file_fd > RESERVED_FDS){
        if(close(p_object->file_fd)){
            errsv=errno;
            BLOCKFRONT_LOG(LOG_CRITICAL,"Error while closing fd %d %s",
                    p_object->file_fd,strerror(errsv));
            ret=OSD_ERROR;
			p_object->file_fd = -1;
            goto end;
        }
    }
    memset(p_object,0,sizeof(object_attr_t));
	p_object->object_index=-1;
end:
    return ret;
}

OSD_RETURN_t 
osd_get_index_to_object(osd_collection_t *p_dc_collection,int obj_index,
        object_attr_t *p_obj_attr){
    OSD_RETURN_t ret = OSD_SUCCESS;
    char start_oid[MAXOIDLEN];
    int counter=0;
    if(!p_dc_collection || !p_obj_attr){
        BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid arguements ");
        ret=OSD_ERROR;
        goto end;
    }
    if(p_dc_collection->start_object==""){
        /*Fetch start object from MDS*/
    }			
    memcpy(start_oid,p_dc_collection->start_object,MAXOIDLEN);


    /*Taking last 4 bytes as counter and adding counter index to last 3 bytes */
    uint32_t start_index=0;
    unsigned char   *p_oct_index=&start_index;
    for(counter=0;counter<4;counter++)
        memcpy((unsigned char *)p_oct_index++,
                (unsigned char *)&start_oid[MAXOIDLEN-counter-1],sizeof(unsigned char));  
    start_index+=obj_index;
    /*Replacing last 4 bytes with object index counter */
	p_oct_index=&start_index;
    for(counter=0;counter<4;counter++)
        memcpy((unsigned char *)&start_oid[MAXOIDLEN-counter-1],
                (unsigned char *)p_oct_index++,sizeof(unsigned char));

    memcpy(p_obj_attr->oid,start_oid,MAXOIDLEN);    
end:
    return ret;

}

OSD_RETURN_t 
osd_recache_object(osd_collection_t *p_dc_collection,object_attr_t *p_object,
        int new_obj_index){
        /*TODO Object caching need to be thread safe distribute fetching to object-namespce based threads */
    OSD_RETURN_t ret = OSD_SUCCESS;
    int errsv=0;
    if(p_object->file_fd > RESERVED_FDS){
        if(close(p_object->file_fd)){
            errsv=errno;
            BLOCKFRONT_LOG(LOG_CRITICAL,"Error while closing fd %d %s",
                    p_object->file_fd,strerror(errsv));
            ret=OSD_ERROR;
            goto end;
        }
    }
    memset(p_object,0,sizeof(object_attr_t));
    if(osd_get_index_to_object(p_dc_collection,new_obj_index,p_object)){
        BLOCKFRONT_LOG(LOG_CRITICAL,"failed to object attrs from index %d ",
                new_obj_index);
        ret=OSD_ERROR;
        goto end;
    }
    char object_path[MAXPATHSIZE]={0};
    if(osd_object_to_dc_path(p_dc_collection->osd_collection_name,p_object,&object_path)){
        char oid_str[MAXOIDSTRLEN]={0};
        print_oid(p_object->oid,oid_str);
        BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get object path %s",
                oid_str);
        ret=OSD_ERROR;
        goto end;		
    }
	if(p_dc_collection->object_size)
		p_object->size=p_dc_collection->object_size;
	else {
		BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get object size ");
        ret=OSD_ERROR;
		/*For variable size objects get from MDS*/
        goto end;
		}
	p_object->object_to_path_fn=osd_object_to_dc_path;
	p_object->ts=time(NULL);
	p_object->object_index=new_obj_index;
	
    /*Intialize file_fd cache for object  */
    struct stat buff;
    if(stat(object_path,&buff)){
        /*Fetch object file from L2*/
        blk_obj_fetch_arg_t fetch_arg;
        memset(&fetch_arg,0,sizeof(blk_obj_fetch_arg_t));
        fetch_arg.object_index=new_obj_index;
        memcpy(fetch_arg.oid,p_object->oid,MAXOIDLEN);
        strncpy(fetch_arg.volume_name,
                p_dc_collection->osd_collection_name,MAXNAMESIZE);
        fetch_arg.object_size=p_object->size;
		fetch_arg.p_dp_map=&(p_dc_collection->dp_data);
		if(get_dpnode_ip_from_index(&(p_dc_collection->dp_data),
			new_obj_index,&(fetch_arg.dp_ip))){
			BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get DP ip object index %d ",
				                                                new_obj_index);
            ret=OSD_ERROR;
            goto end;
			}
        /*Fetch for intial fd-cache */			
        if(simpleOSD_get_object_to_dc(&fetch_arg)){
            BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get dc path for Lun ");
            ret=OSD_ERROR;
            goto end;
        }
    }	
	errsv=0;

    if((p_object->file_fd = open(object_path, 
                p_dc_collection->io_flag))<0){
        errsv=errno;        
        BLOCKFRONT_LOG(LOG_CRITICAL,"unable to open object %s %d %d",
			                         object_path,errsv,strerror(errsv));
        ret=OSD_ERROR;
        goto end;
    }	

end:
    return ret;
}


OSD_RETURN_t 
osd_offset_to_object(osd_collection_t *p_dc_collection,
                                      size_t  offset,
                                      int     *p_object_index,
                                      uint64_t *p_local_offset){

    OSD_RETURN_t ret = OSD_SUCCESS;
    uint64_t     counter=0;
    uint64_t     offset_traversed=0;
    uint64_t     nb_objects=p_dc_collection->nb_objects;
    uint64_t     obj_size=0;

    if(obj_size=p_dc_collection->object_size){
        /*There is fixed size objects */

        if(offset >= nb_objects*obj_size){
            BLOCKFRONT_LOG(LOG_CRITICAL,"\n Offset is beyond the range %lu :%lu",
                    offset,nb_objects*obj_size);
            ret=OSD_ERROR;
            goto end;
        }
        *p_object_index = (offset / obj_size);
        *p_local_offset=offset - (obj_size*(*p_object_index));	
    }else {
        /*TODO if there is variable size objects */
    }
end:
    return ret;
}

OSD_RETURN_t 
osd_op_read(osd_collection_t *p_dc_collection,
        void * buf,size_t buf_len,size_t offset){
    OSD_RETURN_t ret = OSD_SUCCESS;
    int errsv=0;
    uint64_t  buf_read=0;
    int start_object_index=0;
    int end_object_index=0;
    int traverse_index=0;
    uint64_t obj_size=0;

    uint64_t      start_local_offset=0;
    uint64_t      end_local_offset=0;
    uint64_t      local_buf_len=0;
    uint64_t      local_buf_read=0;
    uint64_t      osd_offset=0;



    if(osd_offset_to_object(p_dc_collection,offset,&start_object_index,
                &start_local_offset)){
        BLOCKFRONT_LOG(LOG_CRITICAL,"\n offset to object  failed   %lu ",
                offset);
        ret=OSD_ERROR;
        goto end;
    }
    if(obj_size=p_dc_collection->object_size){
        /*There is fixed size objects */

        if(start_local_offset + buf_len > p_dc_collection->object_size){
            if(osd_offset_to_object(p_dc_collection,offset+buf_len,
                        &end_object_index,
                        &end_local_offset)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n offset to object failed %lu ",
                        offset+buf_len);
                ret=OSD_ERROR;
                goto end;		                                          
            }
        }else{
            end_object_index=start_object_index;
            end_local_offset=start_local_offset;
        }

        traverse_index=start_object_index;
        osd_offset=start_local_offset;

        while(buf_read!=buf_len){
            /*Procedure to identify data/attr for particualr object */
#if RECACHE_FD_ENABLED 
            int current_index=traverse_index%NBMAXOBJECTS;
            object_attr_t *p_current_obj=&(p_dc_collection->p_object_attr_cache[current_index]);


            char object_path[MAXPATHLEN]={0};
            struct stat buff;
            if(osd_object_to_dc_path(p_dc_collection->osd_collection_name,
                        p_current_obj->oid,object_path)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n Error object to path failed %d",
                        traverse_index);
                ret=OSD_ERROR;
                goto end;
            }
            /*Configuration parameters */

            /*Do recache if new object or old object is deleted via eviction etc */
            if((traverse_index!= 
                        p_current_obj->object_index)||(stat(object_path,&buff))){
#else
                object_attr_t local_object,*p_current_obj;
                memset(&local_object,0,sizeof(object_attr_t));
                p_current_obj=&(local_object);
#endif                    
                /*Recaching the current index*/
                if(osd_recache_object(p_dc_collection,p_current_obj,traverse_index)){
                    BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to get object for index  %d ",
                            traverse_index);
                    ret=OSD_ERROR;
                    goto end;		 
                }
#if RECACHE_FD_ENABLED				
            }
#endif

            local_buf_len=(buf_len-buf_read)>=(obj_size-osd_offset)?
                (obj_size-osd_offset):(buf_len-buf_read);
            /* local_buf_len = obj_size or  end_local_offset*/
            /* read data/attr for particualr object has been caluclated */

            /*read from  particualr object */

            if(simpleOSD_obj_op.pfn_offset_read(p_current_obj,
                        (char*)buf+buf_read,local_buf_len,osd_offset)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to read objectfor index  %d ",
                        traverse_index);
                ret=OSD_ERROR;
				if(osd_reset_cache_object(p_current_obj))
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to reset objectfor index  %d ",
                        traverse_index);
                goto end;
            }
            buf_read+=local_buf_len;
            /*Goto next object*/
            traverse_index++;
            osd_offset=0;
#if RECACHE_FD_ENABLED
            /*In case recache fd is enabled dont clean the object */
#else
            if(osd_reset_cache_object(p_current_obj)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to reset objectfor index  %d ",
                        traverse_index);
                ret=OSD_ERROR;
                goto end;

            }
#endif			
        }

    }else {
        /*TODO Future if objects have diferent size*/

    }
end:
    return ret;
}

OSD_RETURN_t 
osd_op_write(osd_collection_t *p_dc_collection,
        void * buf,size_t buf_len,size_t offset){
    OSD_RETURN_t ret = OSD_SUCCESS;
    int errsv=0;
    uint64_t  buf_written=0;
    int start_object_index=0;
    int end_object_index=0;
    int traverse_index=0;
    uint64_t obj_size=0;

    uint64_t      start_local_offset=0;
    uint64_t      end_local_offset=0;
    uint64_t      local_buf_len=0;
    uint64_t      local_buf_write=0;
    uint64_t      osd_offset=0;

    if(osd_offset_to_object(p_dc_collection,offset,&start_object_index,
                &start_local_offset)){
        BLOCKFRONT_LOG(LOG_CRITICAL,"\n offset to object  failed   %lu ",
                offset);
        ret=OSD_ERROR;
        goto end;
    }
    if(obj_size=p_dc_collection->object_size){
        /*There is fixed size objects */

        if(start_local_offset + buf_len > p_dc_collection->object_size){
            if(osd_offset_to_object(p_dc_collection,offset+buf_len,
                        &end_object_index,
                        &end_local_offset)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n offset to object failed %lu ",
                        offset+buf_len);
                ret=OSD_ERROR;
                goto end;		                                          
            }
        }else{
            end_object_index=start_object_index;
            end_local_offset=start_local_offset;
        }
        traverse_index=start_object_index;
        osd_offset=start_local_offset;

        while(buf_written!=buf_len){
#if RECACHE_FD_ENABLED 
            /*Procedure to identify data/attr for particualr object */
            int current_index=traverse_index%NBMAXOBJECTS;
            object_attr_t *p_current_obj=&(p_dc_collection->p_object_attr_cache[current_index]);
			char object_path[MAXPATHLEN]={0};
			struct stat buff;
						if(osd_object_to_dc_path(p_dc_collection->osd_collection_name,
							p_current_obj->oid,object_path)){
							BLOCKFRONT_LOG(LOG_CRITICAL,"\n Error object to path failed %d",
										traverse_index);
								ret=OSD_ERROR;
								goto end;
							}
			/*Do recache if new object or old object is deleted via eviction etc */
            if((traverse_index!= 
                    p_current_obj->object_index)||(stat(object_path,&buff))){
#else
                object_attr_t local_object,*p_current_obj;
                memset(&local_object,0,sizeof(object_attr_t));
                p_current_obj=&(local_object);
#endif                                 
                /*Recaching the current index*/
                if(osd_recache_object(p_dc_collection,p_current_obj,traverse_index)){
                    BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to get  object failed for index  %d ",
                            traverse_index);
                    ret=OSD_ERROR;
                    goto end;		 
                }
#if RECACHE_FD_ENABLED				
            }
#endif				

            local_buf_len=(buf_len-buf_written)>=(obj_size-osd_offset)?(obj_size-osd_offset):(buf_len-buf_written);



            /* local_buf_len = obj_size or  end_local_offset*/
            /* write data/attr for particualr object has been caluclated */

            /*Write to particualr object */
            if(simpleOSD_obj_op.pfn_offset_write(p_current_obj,
                        (char*)buf+buf_written,local_buf_len,osd_offset)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to read objectfor index  %d ",
                        traverse_index);
				if(osd_reset_cache_object(p_current_obj))
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to reset objectfor index  %d ",
                        traverse_index);				
                ret=OSD_ERROR;
                goto end;
            }

            buf_written+=local_buf_len;
            /*Goto next object*/
            traverse_index++;
            osd_offset=0;
#if RECACHE_FD_ENABLED
            /*In case recache fd is enabled dont clean the object */
#else
            if(osd_reset_cache_object(p_current_obj)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"\n unable to reset objectfor index  %d ",
                        traverse_index);
                ret=OSD_ERROR;
                goto end;

            }
#endif					
        }		
    }else{
        /*FUTURE if object are with different size */
    }
end:
    return ret;

}
OSD_RETURN_t 
osd_free_collection(osd_lun_t *p_lun){
    OSD_RETURN_t ret = OSD_SUCCESS;
    osd_collection_t *p_osd_collection = 
        &(p_lun->datacache_info.osd_collection_list[0]);
    int loop=0;
    if(!p_osd_collection){
        BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to free osd collection"
                "Insufficient data");
        ret = OSD_ERROR;
        goto end;
    }
    for(;loop<((p_osd_collection->nb_objects > NBMAXOBJECTS)?
                NBMAXOBJECTS:p_osd_collection->nb_objects);loop++){
        if(p_osd_collection->p_object_attr_cache[loop].file_fd>RESERVED_FDS){
            if((close(p_osd_collection->p_object_attr_cache[loop].file_fd))){
                BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to close file fd %d ",
                        (p_osd_collection->p_object_attr_cache[loop].file_fd));
                ret=OSD_ERROR;
                goto end;
            }
            p_osd_collection->p_object_attr_cache[loop].file_fd=0;
        }
    }
end:
    return ret;
}
OSD_RETURN_t 
osd_init_collection(osd_lun_t *p_lun){
    OSD_RETURN_t ret = OSD_SUCCESS;
    int errsv=0;
    if(!p_lun){
        BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid Lun for osd init");
        ret=OSD_ERROR;
        goto end;		
    }
	p_lun->block_size=4096;
    /*Intialize osd collection */

    osd_collection_t *p_osd_collection=p_lun->datacache_info.osd_collection_list;
    int nb_objects=0;

    p_lun->datacache_info.osd_collection_list[0].io_flag=p_lun->io_flag;
    /*Intialize the cached osd objects */
    /*Need to add LRU*/
    int loop_limit=(p_osd_collection->nb_objects < NBMAXOBJECTS )?p_osd_collection->nb_objects 
        : NBMAXOBJECTS;

    if(p_osd_collection->object_size <=0){
        BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid Object size for Lun %s",p_lun->lun_vol);
        ret=OSD_ERROR;
        goto end;	
    }
#ifndef INTIAL_OBJECT_CACHE	
    goto end;
#endif
    int loop=0;
    for(;loop<loop_limit;loop++){
        object_attr_t *p_obj=&(p_osd_collection->p_object_attr_cache[loop]);
        p_obj->object_index=loop;
        p_obj->ts=time(0);
        if(osd_get_index_to_object(p_osd_collection,loop,p_obj)){
            BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid Object size for Lun %s");
            ret=OSD_ERROR;
            goto end;
        }
        p_obj->size=p_osd_collection->object_size;
        p_obj->object_to_path_fn=osd_object_to_dc_path;

        char obj_path[MAXPATHLEN]={0};
        if(osd_object_to_dc_path(p_osd_collection->osd_collection_name,p_obj->oid,obj_path)){
            BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get dc path for Lun ");
            ret=OSD_ERROR;
            goto end;
        }
        /*Intialize file_fd cache for objects  */
		struct stat buff;
        if(stat(obj_path,&buff)){
            /*Fetch object file from DP*/
            blk_obj_fetch_arg_t fetch_arg;
            memset(&fetch_arg,0,sizeof(blk_obj_fetch_arg_t));
            fetch_arg.object_index=loop;
            memcpy(fetch_arg.oid,p_obj->oid,MAXOIDLEN);
            strncpy(fetch_arg.volume_name,
                    p_osd_collection->osd_collection_name,MAXNAMESIZE);
            fetch_arg.object_size=p_obj->size;
            /*get for intial fd-cache */
			if(get_dpnode_ip_from_index(&(p_osd_collection->dp_data),
			loop,&(fetch_arg.dp_ip))){
			BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get DP ip object index %d ",
				                                                loop);
            ret=OSD_ERROR;
            goto end;
			}
			fetch_arg.p_dp_map=&(p_osd_collection->dp_data);
            if(simpleOSD_get_object_to_dc(&fetch_arg)){
                BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get dc path for Lun ");
                ret=OSD_ERROR;
                goto end;
            }
        }
		
        if((p_obj->file_fd = open(obj_path,p_osd_collection->io_flag))<0){
            errsv=errno;
            BLOCKFRONT_LOG(LOG_CRITICAL,"Error in opening file %s %s ",
                    obj_path,strerror(errsv));
            p_obj->file_fd=-1;
            ret=OSD_ERROR;
            goto end;
        }
    }

    p_lun->block_size=4096;


    goto end;
end:
    return ret;
}

__attribute__((constructor(PRIORITY_INIT_OSD_HELPER_FUNCS))) static void 
                              simpleOSD_osd_helper_constructor(void) {   	
    osd_op_func.pfn_offset_read=osd_op_read;
    osd_op_func.pfn_offset_write=osd_op_write;
    osd_op_func.pfn_offset_to_osd=osd_offset_to_object;
}


