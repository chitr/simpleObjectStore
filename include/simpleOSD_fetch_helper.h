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