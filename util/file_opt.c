/*
 * file_opt.c
 *
 *  Created on: 2017年2月16日
 *      Author: liwei
 */


#include <errno.h>
#include <error.h>
#include <fcntl.h>
#include <unistd.h>
int64_t file_read(int fd, unsigned char *buf, uint64_t count)
{
    uint64_t readbytes, save_count=0;
    for (;;)
    {
        errno= 0;
        readbytes = read(fd, buf+save_count, count-save_count);
        if (readbytes != count-save_count)
        {
            if ((readbytes == 0 || (int) readbytes == -1)&&errno == EINTR)
                continue; /* Interrupted */

            if (readbytes == (size_t) -1)
                return -errno; /* Return with error */
            else if(readbytes==0)
                return save_count;
            else
                save_count += readbytes;
        }
        else
            break;
    }
    return count;
}
int64_t file_write(int fd, unsigned char *buf, uint64_t count)
{
    uint64_t writebytes, save_count=0;
    for (;;)
    {
        errno= 0;
        writebytes = write(fd, buf+save_count, count-save_count);
        if (writebytes != count-save_count)
        {
            if ((writebytes == 0 || (int) writebytes == -1)&&errno == EINTR)
                continue; /* Interrupted */

            if (writebytes == (size_t) -1)
                return -errno; /* Return with error */
            else if(writebytes==0)
                return save_count;
            else
                save_count += writebytes;
        }
        else
            break;
    }
    return save_count;
}
int64_t create_file(const char * filename,unsigned char *buf, uint64_t count)
{
    int fd=0;
    if(check_file_exist(filename)==-1)
        return -1;
    if((fd=open(filename,O_RDWR | O_CREAT, S_IRUSR | S_IWUSR))<=0)
        return -errno;
    if(file_write(fd,buf,count)!=count)
    {
        close(fd);
        return -errno;
    }
    close(fd);
    return 0;
}
/*
 * -1 文件存在
 * 0 文件不存在
 * 其他为错误号
 */
int check_file_exist(const char *filename)
{
   int fd=open(filename,O_RDONLY);
   if(fd>0)
   {
       close(fd);
       return -1;
   }
   else
   {
       if(errno==ENOENT)
       {
           return 0;
       }
       else
       {
           return errno;
       }
   }
}

