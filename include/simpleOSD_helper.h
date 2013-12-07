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
