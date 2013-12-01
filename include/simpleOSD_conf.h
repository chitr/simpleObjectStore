#ifdef USE_LOCAL_CONFIG_PARSER
#include<dictionary.h>
#include<config_parsing.h>
#endif 
#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif
#ifndef ERROR
#define ERROR -1
#endif

#define CONF_LABEL_BLOCK "OSD"
#define MAX_PARAM_LEN 128

#ifdef USE_LOCAL_CONFIG_PARSER
int 
blk_parse_fs(config_file_t in_config,
               blk_frontend_parameters_t *p_blk_param);
#endif
int
blk_parse_conf(char *p_config_path, blk_frontend_parameters_t *p_blk_param);


