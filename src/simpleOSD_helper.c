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
blk_frontend_parameters_t global_blk_frontend_parameters={{0},{0},{0},0,0,0};
char config_path[MAXPATHLEN] = "/etc/simpleOSD/simpleOSD.log";



char dc_path_idenifier[MAX_DC_PATH][MAXNAMESIZE]={
/*osdpath*/"osd_id=",
/*tmppath*/"tmp",
/*Backup_path*/  "osd_id="
};

char dc_file_extention[MAX_DC_PATH][MAXNAMESIZE]={
/*osdpath*/"data",
/*tmppath*/"tmp",
/*Backup_path*/  "bk"
};	


void 
get_dc_parameters_from_conf(blk_frontend_parameters_t *p_dc_parameters){
    /*Get data cache path from config file */
    if(blk_parse_conf(config_path,&global_blk_frontend_parameters)){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Error while parsing conf file,exiting");  
        return;
        BLOCKFRONT_LOG(LOG_CRITICAL,"Error while parsing conf file,Setting defult values");    
        snprintf(p_dc_parameters->dc_path,MAXPATHLEN,"%s","/simpleOSD/");
        snprintf(p_dc_parameters->dp_mds_addr,MAX_PARAM_LEN,"%s","127.0.0.1");
		p_dc_parameters->block_size=4096;
		p_dc_parameters->streaming_size=1048576; /*1MB*/
    }
    p_dc_parameters->is_intiliazed=TRUE;
}

GENERIC_RETURN_t 
oid_to_str(char *p_object_oid,char *p_oid_string){
    GENERIC_RETURN_t ret = GEN_SUCCESS;
	if(!p_object_oid || !p_oid_string){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid arguement");
		ret=GEN_ERROR;
		goto end;
		}
	char *p_oid=(char *)p_object_oid;
	snprintf(p_oid_string,MAXPATHLEN,
		     "%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX",		      
		      p_oid[0],p_oid[1],p_oid[2],p_oid[3],p_oid[4],p_oid[5],p_oid[6],
		      p_oid[7],p_oid[8],p_oid[9],p_oid[10],p_oid[11]);
	end:
		return ret;	
}


GENERIC_RETURN_t 
object_to_dc_path(char *p_vol_identifier,int dcpath_identifier,
	                           char *p_object_oid,char *p_path) {
    GENERIC_RETURN_t ret = GEN_SUCCESS;
	if(!p_vol_identifier || !p_object_oid || !p_path){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid arguement");
		ret=GEN_ERROR;
		goto end;
		}
	char *p_oid=(char *)p_object_oid;
	char oid_str[MAXOIDSTRLEN]="0";
	if(oid_to_str(p_object_oid,oid_str)){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Unable to convert oid to str");
		ret=GEN_ERROR;
		goto end;
		}
	char dir_level_str[DIR_LEVEL_LEN]={0};
	if(DC_PATH_DATA==dcpath_identifier)
	snprintf(dir_level_str,DIR_LEVEL_LEN,"%02hhX/%02hhX",DC_DIR_LEVEL_FIRST(p_oid),
		DC_DIR_LEVEL_SECOND(p_oid));
	else
		snprintf(dir_level_str,DIR_LEVEL_LEN,"/");
	snprintf(p_path,MAXPATHLEN,"%s/%s%s/%s/"
		     "node=%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"
		     ".%s",
		      global_blk_frontend_parameters.dc_path,dc_path_idenifier[dcpath_identifier],
		      p_vol_identifier,dir_level_str,
		      p_oid[0],p_oid[1],p_oid[2],p_oid[3],p_oid[4],p_oid[5],p_oid[6],
		      p_oid[7],p_oid[8],p_oid[9],p_oid[10],p_oid[11],
		      dc_file_extention[dcpath_identifier]);
	end:
		return ret;
}



