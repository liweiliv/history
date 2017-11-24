/*
 * file_opt_windows.c
 *
 *  Created on: 2017年11月23日
 *      Author: liwei
 */
#include "file_opt.h"
#include <winbase.h>
int64_t file_read(FD_TYPE fd, unsigned char *buf, uint64_t count)
{
    DWORD readbytes;
    uint64_t  save_count=0;
    for (;;)
    {
        BOOL rtv = ReadFile(fd, buf + save_count, count - save_count,
                &readbytes, NULL);
        if (!rtv)
        {
            if (readbytes <= 0)
            {
                return save_count;
            }
            else
            {
                save_count += readbytes;
            }
        }
        else
        {
            if (count - save_count != readbytes)
                save_count += readbytes;
            else
                break;

        }
    }
    return count;
}

int64_t file_write(FD_TYPE fd, unsigned char *buf, uint64_t count)
{
    DWORD writebytes;
    uint64_t save_count=0;
    for (;;)
    {
        BOOL rtv = ReadFile(fd, buf + save_count, count - save_count,
                &writebytes, NULL);
        if(!rtv)
        {
            if (writebytes <= 0)
            {
                return save_count;
            }
            else
            {
                save_count += writebytes;
            }
        }
        else
        {
            if (count - save_count != writebytes)
                save_count += writebytes;
            else
                break;
        }
    }
    return save_count;
}
FD_TYPE open_file(const char * file,uint32_t flag)
{
    DWORD   dwDesiredAccess = 0;
    DWORD   dwCreationDisposition = 0;
    if(flag&F_RDONLY)
        dwDesiredAccess|=GENERIC_READ;
    if(flag&F_WRONLY)
        dwDesiredAccess|=GENERIC_WRITE;
    if(flag&F_CREATE)
        dwCreationDisposition = CREATE_NEW;
    else
        dwCreationDisposition = OPEN_EXISTING;
    FD_TYPE fd = CreateFile(file,dwDesiredAccess,FILE_SHARE_READ ,NULL,dwCreationDisposition,FILE_ATTRIBUTE_NORMAL,0);
    if(fd!=INVALID_HANDLE_VALUE )
        return fd;
    else
        return -1;
}
int close_file(FD_TYPE fd)
{
    return CloseHandle(fd);
}
