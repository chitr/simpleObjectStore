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

#include "simpleOSD_bs.h"
#include "simpleOSD_helper.h"
#include "simpleOSD_generic.h"
#include "simpleOSD_conf.h"

extern char logFile[MAXPATHLEN];
extern int  GlobalLogLevel;

extern void  BLOCKFRONT_LOG(int loglevel,char * fmt, ...);

#ifdef USE_LOCAL_CONFIG_PARSER    
int 
blk_parse_param(config_file_t in_config, blk_frontend_parameters_t *p_blk_param) {
    /* parse the fs in the conf file */
    int var_max, var_index;
    char *key_fname;
    char *key_value;
    config_item_t block;
    int rc = 0;

    block = config_FindItemByName(in_config, CONF_LABEL_BLOCK);

    if(block == NULL) {
        BLOCKFRONT_LOG(LOG_CRITICAL,"block is null while parsing fs");
        rc = -1;
        goto end;
    }

    var_max = config_GetNbItems(block);

    for(var_index = 0; var_index < var_max; var_index++) {
        config_item_t item;

        item = config_GetItemByIndex(block, var_index);

        rc = config_GetKeyValue(item, &key_fname, &key_value);

        if(rc) {
            BLOCKFRONT_LOG(LOG_CRITICAL,"get key value is failed");
            goto end;
        }

        if(!strcmp(key_fname, "L2_Mds_Addr")) {
            strncpy(p_blk_param->l2_mds_addr,key_value, MAX_PARAM_LEN);
        } else if(!strcmp(key_fname, "Mds_volDb")) {
            strncpy(p_blk_param->blk_mds_voldb,key_value, MAX_PARAM_LEN);
        } else if(!strcmp(key_fname, "Mds_l2mapDb")) {
            strncpy(p_blk_param->blk_mds_l2mapdb,key_value, MAX_PARAM_LEN);
        } else if(!strcmp(key_fname, "L2_Mds_Port")) {  
            p_blk_param->l2_mds_port=atoi(key_value);
        } else if(!strcmp(key_fname, "Cache_Directory")) {
            strncpy(p_blk_param->dc_path,key_value, MAX_PARAM_LEN);
        } else if(!strcmp(key_fname, "Blocksize")) {  
            p_blk_param->block_size=atoi(key_value);
        } else if(!strcmp(key_fname, "Log_path")) {  
            strncpy(logFile,key_value, MAX_PARAM_LEN);
        } else if(!strcmp(key_fname, "Log_level")) {  
            GlobalLogLevel=atoi(key_value);
        }
    }
    BLOCKFRONT_LOG(LOG_CRITICAL,"Test Log");

end:
    return rc;
}
#endif

int
blk_parse_conf(char *p_config_path,
        blk_frontend_parameters_t *p_blk_param) {
/* TODO Add  implementation specific config parser or use default config parser*/
        int ret=ERROR;		
#ifdef USE_LOCAL_CONFIG_PARSER        
    config_file_t config_struct;
    int rc = 0;

    memset(&config_struct, 0, sizeof(config_struct));
    BLOCKFRONT_LOG(LOG_CRITICAL,"Test Log");
    config_struct = config_ParseFile(p_config_path);
    BLOCKFRONT_LOG(LOG_CRITICAL,"Test Log");
    if(!config_struct) {
        BLOCKFRONT_LOG(LOG_CRITICAL,"failed to construct the config struct,"
                "exiting - %s", p_config_path);
        rc = -1;
        goto end;
    }    

    rc = blk_parse_param(config_struct, p_blk_param);
    if(rc == -1) {
        BLOCKFRONT_LOG(LOG_CRITICAL,"failed to parse the fs block exiting - %s",
                p_config_path);
        rc = -1;
        free(config_struct);
        goto end;
    }
    free(config_struct);
#endif
end:   
    return ret;
}    



