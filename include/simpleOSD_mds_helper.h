
#define VOLUME_NAME  "vol_name"
#define START_OBJECT "start_obj"
#define OBJECT_SIZE  "obj_size"
#define NB_OBJECTS   "Nb_objects"
#define BLOCKSIZE   "blk_size"


typedef enum BLK_MDS_OP{
MDS_GET=0,
MDS_SET
}BLK_MDS_OP_t;


typedef enum mds_blk_datatype{
    BLK_MDS_VOLNAME          = 0x2,
    BLK_MDS_VOLSIZE           = 0x4,
    BLK_MDS_NB_OBJECTS            = 0x8,
    BLK_MDS_OBJECT_SIZE     = 0x10,
    BLK_MDS_BLOCK_SIZE      = 0x20,
    BLK_MDS_START_OID       = 0x40,
    
    BLK_MDS_OBJECT_XTIME  = 0x80,
    BLK_MDS_OBJECT_OID    = 0x100,
    BLK_MDS_OBJECT_INDEX  = 0x200,
    BLK_MDS_DP_MAP        = 0x400,


	BLK_ENTRY_VOL_META     = BLK_MDS_VOLNAME|BLK_MDS_VOLSIZE|BLK_MDS_NB_OBJECTS|
	                         BLK_MDS_OBJECT_SIZE|BLK_MDS_BLOCK_SIZE|BLK_MDS_START_OID,
    BLK_ENTRY_OBJECT_META  = BLK_MDS_OBJECT_OID|BLK_MDS_OBJECT_INDEX|BLK_MDS_OBJECT_XTIME	                         
                               
}mds_blk_datatype_t;


typedef struct blk_mds_args{
BLK_MDS_OP_t op_type;
mds_blk_datatype_t mds_datatype;
char *p_blk_volname;
uint64_t  blk_volSize;
uint64_t  blk_nb_objects;
uint64_t  blk_objectsize;
uint64_t  blk_blocksize;
char      start_oid[MAXOIDLEN];
char      oid[MAXOIDLEN];
uint64_t  xtime;
uint64_t  object_index;
int       is_fs_flag;
dp_data_map_t *p_dp_data_map;
}blk_mds_args_t;


typedef enum MDS_HELPER_RETURN{
MDS_HELPER_SUCCESS=0,
MDS_HELPER_ERROR
}MDS_HELPER_RETURN_t;