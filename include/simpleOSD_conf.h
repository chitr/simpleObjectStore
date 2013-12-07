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


