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
#include <errno.h>
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
#include "simpleOSD_helper.h"

extern blk_frontend_parameters_t global_blk_frontend_parameters;
extern char dc_path_idenifier[MAX_DC_PATH][MAXNAMESIZE];
extern char dc_file_extention[MAX_DC_PATH][MAXNAMESIZE];

void print_oid(char *p_oid,char *p_oid_str){
		int counter=0;		
		for(;counter<MAXOIDLEN;counter++)
			snprintf(p_oid_str,MAXOIDSTRLEN,"%02X",(unsigned char)p_oid[counter]);
		BLOCKFRONT_LOG(LOG_CRITICAL,"OID:%s",p_oid_str);

}

char *
	dc_dir_level(char *p_oid){
	/*If there is provision of multi level dc path return corresponding
	   paths ex.  oid[3]/oid[11]  etc*/
	return "/";
}

OSD_RETURN_t 
	osd_object_to_dc_path(char  *p_dc_collection_name,
	                           char *p_object_oid,char *p_path){
    OSD_RETURN_t ret = OSD_SUCCESS;
	if(!p_dc_collection_name || !p_object_oid || !p_path){
		BLOCKFRONT_LOG(LOG_CRITICAL,"Invalid arguement");
		ret=OSD_ERROR;
		goto end;
		}
	char *p_oid=(char *)p_object_oid;
	snprintf(p_path,MAXPATHLEN,"%s/%s%s/%02hhX/%02hhX/"
		     "node=%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX%02hhX"
		     ".%s",
		      global_blk_frontend_parameters.dc_path,
		      dc_path_idenifier[DC_PATH_DATA],p_dc_collection_name,DC_DIR_LEVEL_FIRST(p_oid),
		      DC_DIR_LEVEL_SECOND(p_oid),
		      p_oid[0],p_oid[1],p_oid[2],p_oid[3],p_oid[4],p_oid[5],p_oid[6],
		      p_oid[7],p_oid[8],p_oid[9],p_oid[10],p_oid[11],
		      dc_file_extention[DC_PATH_DATA]);
	end:
		return ret;
}



