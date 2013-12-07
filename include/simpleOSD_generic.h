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
#ifndef SUPPORT_UNFORMATTED_DISK
#define SUPPORT_UNFORMATTED_DISK TRUE
#endif
#define LOG_SIZE 4096

#ifndef SUCCESS
#define SUCCESS 0
#endif

#ifndef MAX_IP_LEN
#define MAX_IP_LEN 25
#endif

#ifndef LOG_DEBUG
#define LOG_DEBUG    0
#endif
#ifndef LOG_INFO
#define LOG_INFO     1
#endif
#ifndef LOG_CRITICAL
#define LOG_CRITICAL 2
#endif
#ifndef LOG_ERROR
#define LOG_ERROR    3
#endif
#ifndef LOGTYPECOUNT
#define LOGTYPECOUNT 4
#endif


#ifndef MAX_PARAM_LEN
#define MAX_PARAM_LEN 128
#endif

#ifndef MAXLOGTYPELEN
#define MAXLOGTYPELEN 25
#endif

#ifndef MAXPATHLEN
#define MAXPATHLEN 4096
#endif



#ifndef MAX_MSG_LEN
#define MAX_MSG_LEN 4096
#endif



#define RESERVED_FDS 3
/*Could be changed to multi level directories */
#define FIRST_LEVEL  11
#define SECOND_LEVEL 10
#define DC_DIR_LEVEL(oid)  "/"
#define DC_DIR_LEVEL_FIRST(oid)  oid[FIRST_LEVEL]
#define DC_DIR_LEVEL_SECOND(oid) oid[SECOND_LEVEL]

