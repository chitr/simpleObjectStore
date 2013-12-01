#ifndef DEFAULT_LOCAL_MDS_PORT
#define DEFAULT_LOCAL_MDS_PORT 30000
#endif

#ifndef DIR_LEVEL_LEN
#define DIR_LEVEL_LEN 25
#endif 

typedef enum GENERIC_RETURN{
GEN_SUCCESS=0,
GEN_ERROR	
}GENERIC_RETURN_t;

typedef enum DC_PATH {
DC_PATH_DATA=0,
DC_PATH_FETCH,
DC_PATH_BACKUP,
MAX_DC_PATH
}DC_PATH;
