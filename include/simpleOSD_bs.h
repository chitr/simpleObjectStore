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

#ifndef BLOCK_FRONTEND
#define BLOCK_FRONTEND 
#endif


#ifndef MAXNAMESIZE
#define MAXNAMESIZE 256
#endif
#ifndef MAXPATHSIZE
#define MAXPATHSIZE 4096
#endif
#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif

#ifndef NBMAXOBJECTS 
#define NBMAXOBJECTS 1000
#endif

#ifndef MAXOIDLEN
#define MAXOIDLEN 12
#endif
#ifndef MAXOIDSTRLEN
#define MAXOIDSTRLEN 12*2+1
#endif

#ifndef MAX_PARAM_LEN
#define MAX_PARAM_LEN 128
#endif

#ifndef MAX_IP_LEN
#define MAX_IP_LEN 25
#endif

#ifndef MAX_IP_STRLEN
#define MAX_IP_STRLEN 25
#endif

#define LUN_FREE  0 
#define LUN_USED  1 


/*Configuration parameters */
#define RECACHE_FD_ENABLED FALSE


#define GET_MEMORY malloc
/*Using last 3 bits of pointers to store LUN status i.e if it is free or used 
   using only 2 states out of 7 possible states :)*/

#define VAL_POINTER(p) (unsigned long)(0xffffffffffffff8UL & (unsigned long)(p))
#define VAL_DATA(p) (unsigned long)(7 & (unsigned long)(p))

#define  FREE_LUN(p) \
        p = (0xffffffffffffff8UL & (unsigned long)(p));\
        p = ( 0 | (unsigned long)(p)) ; 

#define USE_LUN(p) \
        p = (0xffffffffffffff8UL & (unsigned long)(p));\
        p = ( 1 | (unsigned long)(p)) ;
        

#define IS_FREE(p) \
!(1 & (unsigned long)(p))

#define PRIORITY_RESERVED 100      
#define PRIORITY_INIT_OSD_HELPER_FUNCS    PRIORITY_RESERVED + 1
#define PRIORITY_INIT_BLK_FUNCS           PRIORITY_RESERVED + 2
#define PRIORITY_INIT_EIGHTKKPC_LUN_LIST  PRIORITY_RESERVED + 3
#define PRIORITY_INIT_BS_FUNCS            PRIORITY_RESERVED + 4
#define PRIORITY_INIT_OBJ_FUNCS           PRIORITY_RESERVED + 5

/***************START dp (dataplacement) helper ********************************************/
/*32 bit ip address corresponds to 1 byte(8bit) for each octet*/
typedef union IP { 
    uint32_t uint_ip; 
    struct {
        unsigned char first_octet;
        unsigned char second_octet;
        unsigned char third_octet;
        unsigned char fourth_octet;
    } octet_ip;                                  
}IP;

#ifndef NB_MAX_dp
#define NB_MAX_dp 100
#endif


typedef struct dp_ip_obj_range {
int start_oid;
IP  dp_ip;
int dp_id;
int range;
}dp_ip_obj_range_t;

typedef struct dp_data_map{
dp_ip_obj_range_t dp_data[NB_MAX_dp];	
int               nb_dp_nodes;
int               fixed_range;
void*             *dp_node_list;
}dp_data_map_t;
/***************END DP helper *********************************************/


/**************** START :headers for simpleOSD bs & io ****************************/

/*Setting simpleOSD blk io in/out args */
//#define SET_READ_IN_ARG(lun_fd) ""


typedef enum BS_IO_RETURN{
BS_IO_SUCCESS=0,
BS_IO_ERROR
}BS_IO_RETURN_t;

typedef enum BLK_IO_RETURN{
BLK_IO_SUCCESS=0,
BLK_IO_ERROR
}BLK_IO_RETURN_t;

typedef enum bs_op_type{
BLK_READ=0,
BLK_WRITE,
BLK_ADVICE
}bs_op_type_t;

typedef struct blk_io_args{
int  lun_index;
void *buf;
size_t buf_len;
size_t offset;
int    posix_advice;
int    errorsv;
}blk_io_args_t;

typedef struct blk_io_op{
BLK_IO_RETURN_t (*pfn_read64)(void * p_read_in,void * p_read_out);
BLK_IO_RETURN_t (*pfn_write64)(void * p_write_in,void *p_write_out);
BLK_IO_RETURN_t (*pfn_posix_advise)(void * p_posix_advise_in,
	                                       void *p_posix_advise_out);
BLK_IO_RETURN_t (*pfn_unmap_file_region)(void *p_unmap_in,
	                                              void *p_unmap_out);
BLK_IO_RETURN_t (*pfn_data_sync)(void *p_data_sync_in,
	                                          void *p_data_sync_out);
}blk_io_op_t;

typedef struct bs_io_op{
BS_IO_RETURN_t (*pfn_read64)
	            (int lun_fd,void *p_read_buf,size_t buf_len,size_t offset);
BS_IO_RETURN_t (*pfn_write64)
	            (int lun_fd,void *p_read_buf,size_t buf_len,size_t offset);
BS_IO_RETURN_t (*pfn_posix_advise)
	             (int lun_fd,off_t offset,off_t len,int advice);
BS_IO_RETURN_t (*pfn_unmap_file_region)(
	             int lun_fd,off_t offset,off_t len);
BS_IO_RETURN_t (*pfn_data_sync)(lun_fd);
BS_IO_RETURN_t (*pfn_open)
	           (char *p_path,int oflag,uint64_t *p_size,uint32_t *p_blksize,
	            int *p_lun_fd);
BS_IO_RETURN_t (*pfn_close)(int lun_fd);
}bs_io_op_t;


/**************** END:headers for simpleOSD bs & io *******************************/




/****************START:header for simpleOSD osd *********************************/
typedef enum OSD_RETURN{
OSD_SUCCESS=0,
OSD_ERROR
}OSD_RETURN_t;

typedef struct object_attr{
	char oid[MAXOIDLEN]; /*OID for dc object*/
	uint64_t ts;         /*last timestamp for modification*/
	uint64_t size;
	int object_index;
	int file_fd;
	OSD_RETURN_t (*object_to_path_fn)(char *p_col_name,char *p_obj_id,
		                           char *p_path);/*object to dc path*/
}object_attr_t;

typedef struct osd_collection{
	object_attr_t p_object_attr_cache[NBMAXOBJECTS]; /*Only NBMAXOBJECTS attributes */
	                                          /*are cached for   nb_objects       */
											  /*Need to add efficient  caching layer */
	size_t   object_size;                     /*For fixed size objects */										  
	uint64_t nb_objects;
	char     osd_collection_name[MAXNAMESIZE];
	int      io_flag;              /*Local manipulation wrt LUN io flag could be done here  */
	char     start_object[MAXOIDLEN];
	dp_data_map_t dp_data;
OSD_RETURN_t (*cache_hash_fn)(uint64_t *p_object_index,
		                      uint64_t *p_hash_index,void *additional_info);		
}osd_collection_t;

#define INIT_OSD_COLLECTION(p_osd_collection_entry,nb_objects,hash_fn) \
	memset(p_osd_collection_entry,0,sizeof(osd_collection_t)); \
	p_osd_collection_entry->nb_objects = nb_objects; \
	p_osd_collection_entry->cache_hash_fn = hash_fn; 


/*map nb_objects to NBMAXOBJECTS objects */	
OSD_RETURN_t 
cache_hash_index_function(uint64_t *p_object_index,
	                                            uint64_t *p_hash_index);	


/****************START:headers for simpleOSD osd helper ***************************/

#ifndef MAXCOLLECTIONSIZE
#define MAXCOLLECTIONSIZE 1
#endif

#ifndef NB_MAX_LUN
#define NB_MAX_LUN 20
#endif

typedef struct lun_dc_info {
	char lun_dc_path[MAXPATHSIZE];
	osd_collection_t osd_collection_list[MAXCOLLECTIONSIZE]; /*For current implemenation there*/
	                                        /* is single  collection for a LUN   */	
}lun_dc_info_t;

/*This is how OSD information organised for each LUN */
typedef struct osd_lun{	
    int lun_index;
	int io_flag;
	uint64_t block_size;
	char lun_vol[MAXNAMESIZE];/*misnomer for osd device name*/
	uint64_t lun_capacity;
	int      is_fs_done;      /*This flag is false if there is no file system*/
	void * p_mds_info;
	lun_dc_info_t datacache_info;	
}osd_lun_t;



typedef struct osd_helper_func{
OSD_RETURN_t (*pfn_offset_to_osd)
	(osd_collection_t *p_dc_collection,size_t  offset,int *p_object_index,uint64_t *p_local_offset);
OSD_RETURN_t (*pfn_offset_read)
	(osd_collection_t *p_dc_collection,void * buf,size_t buf_len,size_t offset);
OSD_RETURN_t (*pfn_offset_write)
	(osd_collection_t *p_dc_collection,void * buf,size_t buf_len,size_t offset);
}osd_helper_func_t;


/****************END:headers for simpleOSD osd helper ***************************/

/****************START: headers for simpleOSD object operation *******************/
typedef enum OBJ_RETURN{
OBJ_SUCCESS=0,
OBJ_ERROR
}OBJ_RETURN_t;

typedef struct obj_op_func{
OBJ_RETURN_t (*pfn_offset_read)
	         (object_attr_t *p_obj,void *buf,uint64_t buf_len,uint64_t offset);
OBJ_RETURN_t (*pfn_offset_write)
              (object_attr_t *p_obj,void *buf,uint64_t buf_len,uint64_t offset);
}obj_op_func_t;



/****************END: headers for simpleOSD object operation *******************/

/****************START headers for simpleOSD global parameters *****************/

typedef struct blk_frontend_parameters {
char dc_path[MAXPATHLEN];
char dp_mds_addr[MAX_PARAM_LEN]; 
int  block_size;
uint64_t streaming_size;
int  is_intiliazed;
}blk_frontend_parameters_t;


