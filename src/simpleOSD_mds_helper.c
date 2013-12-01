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
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>



#include <linux/fs.h>
#include <sys/epoll.h>

#include "simpleOSD_bs.h"
#include "simpleOSD_generic.h"
#include "simpleOSD_mds_helper.h"

#define TEST_NB_OBJECTS  1000
#define TEST_SIZE_OBJECT 1048576;/*10MB*/
#define TEST_DP_NODES    2
#define TEST_DP_NODE_IP  "127.0.0.1"

extern blk_frontend_parameters_t global_blk_frontend_parameters;
MDS_HELPER_RETURN_t 
get_dpnode_ip_from_index(dp_data_map_t *p_dp_map,int object_index,IP *p_dp_ip){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;
    int loop=0;
    if((!p_dp_map) || (!p_dp_ip)){
        BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid arguement for mds ip from index");
        ret=MDS_HELPER_ERROR;
        goto end;
    }
    for(;loop<p_dp_map->nb_dp_nodes;loop++){
        dp_ip_obj_range_t *p_obj=&(p_dp_map->dp_data[loop]);
        if((object_index>=p_obj->start_oid) &&
                (object_index<=(p_obj->start_oid +p_obj->range))){
            *p_dp_ip=p_obj->dp_ip;
            BLOCKFRONT_LOG(LOG_DEBUG,"Index :%d DPip %x",object_index,*p_dp_ip);
            goto end;
        }
    }
    BLOCKFRONT_LOG(LOG_CRITICAL,"Index :%d DPip not found ",object_index);
    ret=MDS_HELPER_ERROR;

end:
    return ret;
}

MDS_HELPER_RETURN_t 
mds_get_blk_device_data(blk_mds_args_t *p_data_in,
        blk_mds_args_t *p_data_out){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;
    int errsv=0;
     /*TODO : Add implementation specific api to get data/configuration from db or config file*/
end:
    return ret;
}


MDS_HELPER_RETURN_t 
mds_generate_test_data(blk_mds_args_t *p_data_in,
        blk_mds_args_t *p_data_out){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;

    if(p_data_out->mds_datatype & BLK_MDS_START_OID){
        char oid[MAXOIDLEN];	
        /*Genrate start oid */
        memset(oid,0,MAXOIDLEN);
        uint64_t cur_time=time(NULL);
        memcpy(&oid[1],&cur_time,sizeof(uint64_t));
        memcpy(p_data_out->start_oid,oid,MAXOIDLEN);	
    }
    if(p_data_out->mds_datatype & BLK_MDS_NB_OBJECTS){

        p_data_out->blk_nb_objects=100/*(rand()%1000 )+ 1*/;

    }
    if(p_data_out->mds_datatype & BLK_MDS_OBJECT_SIZE){
        p_data_out->blk_objectsize=1048576;/*10MB*/
    }
    if(p_data_out->mds_datatype & BLK_MDS_VOLSIZE){
        p_data_out->blk_volSize=
            (p_data_out->blk_objectsize * p_data_out->blk_nb_objects);
    }
    if(p_data_out->mds_datatype & BLK_MDS_BLOCK_SIZE){
        p_data_out->blk_blocksize=4096;
    }
	int loop=0;
    int fixed_range=0;
    while( loop < TEST_DP_NODES ) { 
		int errsv=0;
        dp_ip_obj_range_t *p_dp_obj=&(p_data_out->p_dp_data_map->dp_data[loop]);		
            p_dp_obj->start_oid=loop*TEST_NB_OBJECTS/TEST_DP_NODES;
            loop++;
            p_dp_obj->range=TEST_NB_OBJECTS/TEST_DP_NODES;
            if(!fixed_range)
                fixed_range=p_dp_obj->range;        
  
            char dp_node_ip_str[MAX_IP_STRLEN]={0};
            strncpy(dp_node_ip_str,TEST_DP_NODE_IP,MAX_IP_STRLEN);
            if(!inet_pton(AF_INET,dp_node_ip_str,(char *)&(p_dp_obj->dp_ip))){				
                errsv=errno;
                BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid ip %s:%s",dp_node_ip_str,strerror(errsv));
                ret=MDS_HELPER_ERROR;
                goto end;	
            }            
    }
    p_data_out->p_dp_data_map->nb_dp_nodes=loop;
    p_data_out->p_dp_data_map->fixed_range=fixed_range;
end:
    return ret;	
}


MDS_HELPER_RETURN_t
mds_get_blk_device_info(blk_mds_args_t *p_data_in,blk_mds_args_t *p_data_out){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;
#ifdef 	GET_INFO_FROM_MDS_DB
    if(mds_generate_test_data(p_data_in,p_data_out)){
#else
    if(mds_get_blk_device_data(p_data_in,p_data_out)){
#endif
        BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to get block device data from MDS :%s",
                p_data_in->p_blk_volname);
        ret=MDS_HELPER_ERROR;
        goto end;
    }
    if(p_data_out->mds_datatype & BLK_MDS_VOLSIZE){
        p_data_out->blk_volSize=
            (p_data_out->blk_objectsize * p_data_out->blk_nb_objects);
    }
    if(p_data_out->mds_datatype & BLK_MDS_BLOCK_SIZE){

        p_data_out->blk_blocksize=global_blk_frontend_parameters.block_size;
    }

end:
    return ret;
}
MDS_HELPER_RETURN_t
mds_get_object_info(blk_mds_args_t *p_data_in,blk_mds_args_t *p_data_out){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;


end:
    return ret;

}
MDS_HELPER_RETURN_t
mds_set_blk_device_info(blk_mds_args_t *p_data_in,blk_mds_args_t *p_data_out){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;


end:
    return ret;

}
MDS_HELPER_RETURN_t
mds_set_object_info(blk_mds_args_t *p_data_in,blk_mds_args_t *p_data_out){
    MDS_HELPER_RETURN_t ret=MDS_HELPER_SUCCESS;


end:
    return ret;

}
	

