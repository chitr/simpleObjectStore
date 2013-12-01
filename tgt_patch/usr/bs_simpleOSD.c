
/*
 * SimpleOSD  backend prototype
 *
 */
#define _XOPEN_SOURCE 600

#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include <linux/fs.h>
#include <sys/epoll.h>

#include "list.h"
#include "util.h"
#include "tgtd.h"
#include "scsi.h"
#include "spc.h"
#include "bs_thread.h"

#include <simpleOSD_bs.h>


extern bs_io_op_t simpleOSD_bs_op;

static void set_medium_error(int *result, uint8_t *key, uint16_t *asc) {
    *result = SAM_STAT_CHECK_CONDITION;
    *key = MEDIUM_ERROR;
    *asc = ASC_READ_ERROR;
}

static void bs_sync_sync_range(struct scsi_cmd *cmd, uint32_t length,
        int *result, uint8_t *key, uint16_t *asc) {
    int ret;

    ret = simpleOSD_bs_op.pfn_data_sync(cmd->dev->fd);
    if (ret)
        set_medium_error(result, key, asc);
}

static void 
bs_simpleOSD_request(struct scsi_cmd *cmd){
    int ret, fd = cmd->dev->fd;
    uint32_t length;
    int result = SAM_STAT_GOOD;
    uint8_t key;
    uint16_t asc;
    uint32_t info = 0;
    char *tmpbuf;
    size_t blocksize;
    uint64_t offset = cmd->offset;
    uint32_t tl     = cmd->tl;
    int do_verify = 0;
    int i;
    char *ptr;
    const char *write_buf = NULL;
    ret = length = 0;
    key = asc = 0;

    switch (cmd->scb[0]){
        case ORWRITE_16:
            length = scsi_get_out_length(cmd);

            tmpbuf = malloc(length);
            if (!tmpbuf) {
                result = SAM_STAT_CHECK_CONDITION;
                key = HARDWARE_ERROR;
                asc = ASC_INTERNAL_TGT_FAILURE;
                break;
            }

            //ret = pread64(fd, tmpbuf, length, offset);
            if(simpleOSD_bs_op.pfn_read64(fd,tmpbuf, length, offset))
				ret=-1;
			else
				ret=length;

            if (ret != length) {
                set_medium_error(&result, &key, &asc);
                free(tmpbuf);
                break;
            }

            ptr = scsi_get_out_buffer(cmd);
            for (i = 0; i < length; i++)
                ptr[i] |= tmpbuf[i];

            free(tmpbuf);

            write_buf = scsi_get_out_buffer(cmd);
            goto write;
        case COMPARE_AND_WRITE:
            /* Blocks are transferred twice, first the set that
             * we compare to the existing data, and second the set
             * to write if the compare was successful.
             */
            length = scsi_get_out_length(cmd) / 2;
            if (length != cmd->tl) {
                result = SAM_STAT_CHECK_CONDITION;
                key = ILLEGAL_REQUEST;
                asc = ASC_INVALID_FIELD_IN_CDB;
                break;
            }

            tmpbuf = malloc(length);
            if (!tmpbuf) {
                result = SAM_STAT_CHECK_CONDITION;
                key = HARDWARE_ERROR;
                asc = ASC_INTERNAL_TGT_FAILURE;
                break;
            }

            if(simpleOSD_bs_op.pfn_read64(fd,tmpbuf, length, offset))
				ret=-1;
			else
				ret=length;

            if (ret != length) {
                set_medium_error(&result, &key, &asc);
                free(tmpbuf);
                break;
            }

            if (memcmp(scsi_get_out_buffer(cmd), tmpbuf, length)) {
                uint32_t pos = 0;
                char *spos = scsi_get_out_buffer(cmd);
                char *dpos = tmpbuf;

                /*
                 * Data differed, this is assumed to be 'rare'
                 * so use a much more expensive byte-by-byte
                 * comparasion to find out at which offset the
                 * data differs.
                 */
                for (pos = 0; pos < length && *spos++ == *dpos++;
                        pos++)
                    ;
                info = pos;
                result = SAM_STAT_CHECK_CONDITION;
                key = MISCOMPARE;
                asc = ASC_MISCOMPARE_DURING_VERIFY_OPERATION;
                free(tmpbuf);
                break;
            }

            if (cmd->scb[1] & 0x10)
                simpleOSD_bs_op.pfn_posix_advise(fd, offset, length,
                        POSIX_FADV_NOREUSE);

            free(tmpbuf);

            write_buf = scsi_get_out_buffer(cmd) + length;
            goto write;
        case SYNCHRONIZE_CACHE:
        case SYNCHRONIZE_CACHE_16:
            /* TODO */
            length = (cmd->scb[0] == SYNCHRONIZE_CACHE) ? 0 : 0;

            if (cmd->scb[1] & 0x2) {
                result = SAM_STAT_CHECK_CONDITION;
                key = ILLEGAL_REQUEST;
                asc = ASC_INVALID_FIELD_IN_CDB;
            } else
                bs_sync_sync_range(cmd, length, &result, &key, &asc);
            break;
        case WRITE_VERIFY:
        case WRITE_VERIFY_12:
        case WRITE_VERIFY_16:
            do_verify = 1;
        case WRITE_6:
        case WRITE_10:
        case WRITE_12:
        case WRITE_16:
            length = scsi_get_out_length(cmd);
            write_buf = scsi_get_out_buffer(cmd);
write:
            if(simpleOSD_bs_op.pfn_write64(fd,write_buf, length,
                    offset))
                    ret=-1;
			else
				ret=length;

            if (ret == length) {
                struct mode_pg *pg;

                /*
                 * it would be better not to access to pg
                 * directy.
                 */
                pg = find_mode_page(cmd->dev, 0x08, 0);
                if (pg == NULL) {
                    result = SAM_STAT_CHECK_CONDITION;
                    key = ILLEGAL_REQUEST;
                    asc = ASC_INVALID_FIELD_IN_CDB;
                    break;
                }
                if (((cmd->scb[0] != WRITE_6) && (cmd->scb[1] & 0x8)) ||
                        !(pg->mode_data[0] & 0x04))
                    bs_sync_sync_range(cmd, length, &result, &key,
                            &asc);
            } else
                set_medium_error(&result, &key, &asc);

            if ((cmd->scb[0] != WRITE_6) && (cmd->scb[1] & 0x10))
                simpleOSD_bs_op.pfn_posix_advise(fd, offset, length,
                        POSIX_FADV_NOREUSE);
            if (do_verify)
                goto verify;
            break;
        case WRITE_SAME:
        case WRITE_SAME_16:
            /* WRITE_SAME used to punch hole in file */
            if (cmd->scb[1] & 0x08) {
                ret = -1 ; //ret = unmap_file_region(fd, offset, tl); TODO
                if (ret != 0) {
                    eprintf("Failed to punch hole for WRITE_SAME"
                            " command\n");
                    result = SAM_STAT_CHECK_CONDITION;
                    key = HARDWARE_ERROR;
                    asc = ASC_INTERNAL_TGT_FAILURE;
                    break;
                }
                break;
            }
            while (tl > 0) {
                blocksize = 1 << cmd->dev->blk_shift;
                tmpbuf = scsi_get_out_buffer(cmd);

                switch(cmd->scb[1] & 0x06) {
                    case 0x02: /* PBDATA==0 LBDATA==1 */
                        put_unaligned_be32(offset, tmpbuf);
                        break;
                    case 0x04: /* PBDATA==1 LBDATA==0 */
                        /* physical sector format */
                        put_unaligned_be64(offset, tmpbuf);
                        break;
                }

                if(simpleOSD_bs_op.pfn_write64(fd,tmpbuf, blocksize, offset))
					ret=-1;
				else
					ret=blocksize;
                if (ret != blocksize)
                    set_medium_error(&result, &key, &asc);

                offset += blocksize;
                tl     -= blocksize;
            }
            break;
        case READ_6:
        case READ_10:
        case READ_12:
        case READ_16:
            length = scsi_get_in_length(cmd);
            if(simpleOSD_bs_op.pfn_read64(fd, scsi_get_in_buffer(cmd), length,
                    offset))
                    ret=-1;
			else
				ret=length;

            if (ret != length)
                set_medium_error(&result, &key, &asc);

            if ((cmd->scb[0] != READ_6) && (cmd->scb[1] & 0x10))
                simpleOSD_bs_op.pfn_posix_advise(fd, offset, length,
                        POSIX_FADV_NOREUSE);

            break;
        case PRE_FETCH_10:
        case PRE_FETCH_16:
            ret = simpleOSD_bs_op.pfn_posix_advise(fd, offset, cmd->tl,
                    POSIX_FADV_WILLNEED);

            if (ret != 0)
                set_medium_error(&result, &key, &asc);
            break;
        case VERIFY_10:
        case VERIFY_12:
        case VERIFY_16:
verify:
            length = scsi_get_out_length(cmd);

            tmpbuf = malloc(length);
            if (!tmpbuf) {
                result = SAM_STAT_CHECK_CONDITION;
                key = HARDWARE_ERROR;
                asc = ASC_INTERNAL_TGT_FAILURE;
                break;
            }

            if(simpleOSD_bs_op.pfn_read64(fd,tmpbuf, length, offset))
				ret=-1;
			else
				ret=length;

            if (ret != length)
                set_medium_error(&result, &key, &asc);
            else if (memcmp(scsi_get_out_buffer(cmd), tmpbuf, length)) {
                result = SAM_STAT_CHECK_CONDITION;
                key = MISCOMPARE;
                asc = ASC_MISCOMPARE_DURING_VERIFY_OPERATION;
            }

            if (cmd->scb[1] & 0x10)
                simpleOSD_bs_op.pfn_posix_advise(fd, offset, length,
                        POSIX_FADV_NOREUSE);

            free(tmpbuf);
            break;
        case UNMAP:
            if (!cmd->dev->attrs.thinprovisioning) {
                result = SAM_STAT_CHECK_CONDITION;
                key = ILLEGAL_REQUEST;
                asc = ASC_INVALID_FIELD_IN_CDB;
                break;
            }

            length = scsi_get_out_length(cmd);
            tmpbuf = scsi_get_out_buffer(cmd);

            if (length < 8)
                break;

            length -= 8;
            tmpbuf += 8;

            while (length >= 16) {
                offset = get_unaligned_be64(&tmpbuf[0]);
                offset = offset << cmd->dev->blk_shift;

                tl = get_unaligned_be32(&tmpbuf[8]);
                tl = tl << cmd->dev->blk_shift;

                if (offset + tl > cmd->dev->size) {
                    eprintf("UNMAP beyond EOF\n");
                    result = SAM_STAT_CHECK_CONDITION;
                    key = ILLEGAL_REQUEST;
                    asc = ASC_LBA_OUT_OF_RANGE;
                    break;
                }

                if (tl > 0) {
                    if (simpleOSD_bs_op.pfn_unmap_file_region(fd, offset, tl) != 0) {
                        eprintf("Failed to punch hole for"
                                " UNMAP at offset:%" PRIu64
                                " length:%d\n",
                                offset, tl);
                        result = SAM_STAT_CHECK_CONDITION;
                        key = HARDWARE_ERROR;
                        asc = ASC_INTERNAL_TGT_FAILURE;
                        break;
                    }
                }

                length -= 16;
                tmpbuf += 16;
            }
            break;
        default:
            break;
    }

    dprintf("\n simpleOSD io done %p %x %d %u\n", cmd, cmd->scb[0], ret, length);

    scsi_set_result(cmd, result);

    if (result != SAM_STAT_GOOD) {
        eprintf("io error %p %x %d %d %" PRIu64 ", %m\n",
                cmd, cmd->scb[0], ret, length, offset);
        sense_data_build(cmd, key, asc);
    }
}
static int 
bs_simpleOSD_open(struct scsi_lu *lu, char *path, int *p_fd, uint64_t *p_size){
    uint32_t blksize = 0; 
    BLK_IO_RETURN_t ret = BLK_IO_SUCCESS;
    int bs_open_return=-1;

    if(ret=simpleOSD_bs_op.pfn_open(path, O_RDWR|O_LARGEFILE|lu->bsoflags, p_size, 
                &blksize,p_fd)){
        dprintf("\nError from simpleOSD_lun_open  \n");
        goto end;		
    }
    /* If we get access denied, try opening the file in readonly mode */
    if (*p_fd < 0  && (errno == EACCES || errno == EROFS)) {
        if(simpleOSD_bs_op.pfn_open(path, O_RDWR|O_LARGEFILE|lu->bsoflags,
                    p_size, &blksize,p_fd))
        {
            dprintf("\nError from simpleOSD_lun_open  \n");
            goto end;
        }
        lu->attrs.readonly = 1;
    }
    if (*p_fd < 0)
        goto end;

    if (!lu->attrs.no_auto_lbppbe)
        update_lbppbe(lu, blksize);
    bs_open_return = TGTADM_SUCCESS;
    dprintf("\neighkpc open succesful \n");
end:
    return bs_open_return;
}

static void 
bs_simpleOSD_close(struct scsi_lu *lu){
    BLK_IO_RETURN_t ret = BLK_IO_SUCCESS;
    int bs_close_return=-1;
    if(ret=simpleOSD_bs_op.pfn_close(lu->fd))
        dprintf("\neighkpc close failed:%d \n",lu->fd);	
    else 
        dprintf("\neighkpc close Successful:%d \n",lu->fd);		
}

static tgtadm_err 
bs_simpleOSD_init(struct scsi_lu *lu){
    struct bs_thread_info *info = BS_THREAD_I(lu);
    return bs_thread_open(info, bs_simpleOSD_request, nr_iothreads);
}

static void 
bs_simpleOSD_exit(struct scsi_lu *lu){
    struct bs_thread_info *info = BS_THREAD_I(lu);
    bs_thread_close(info);
}

static struct backingstore_template simpleOSD_bst = {
    .bs_name		= "simpleOSD",
    .bs_datasize	        = sizeof(struct bs_thread_info),
    .bs_open		= bs_simpleOSD_open,
    .bs_close		= bs_simpleOSD_close,
    .bs_init		= bs_simpleOSD_init,
    .bs_exit		= bs_simpleOSD_exit,
    .bs_cmd_submit		= bs_thread_cmd_submit,
    .bs_oflags_supported    = O_SYNC | O_DIRECT,
};

__attribute__((constructor)) static void bs_simpleOSD_constructor(void) {   
    register_backingstore_template(&simpleOSD_bst);
}
