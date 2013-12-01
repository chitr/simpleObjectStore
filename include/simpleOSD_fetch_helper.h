typedef enum BLK_OBJ_FETCH_ret{
BLK_OBJ_FETCH_SUCCESS=0,
BLK_OBJ_FETCH_ERROR	
}BLK_OBJ_FETCH_ret;

typedef struct blk_obj_fetch_arg{
	char volume_name[MAXNAMESIZE];
	char oid[MAXOIDLEN];
	char *p_oid_str;
	uint64_t object_index;
	uint64_t object_size;
	dp_data_map_t *p_dp_map;
	IP       dp_ip;
}blk_obj_fetch_arg_t;