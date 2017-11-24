/*
 * file_opt.h
 *
 *  Created on: 2017年2月14日
 *      Author: liwei
 */

#ifndef LIB_UTIL_FILE_OPT_H_
#define LIB_UTIL_FILE_OPT_H_
//todo test
#define _LINUX
#include <stdint.h>
#ifdef _WINDOWS
typedef HANDLE  FD_TYPE;
#endif
#ifdef _LINUX
typedef int32_t  FD_TYPE;
#endif
#define F_RDONLY 0x01
#define F_WRONLY 0x02
#define F_RDWR (F_RDONLY|F_WRONLY)
#define F_CREATE 0x04
#ifdef __cplusplus
extern "C"
{
#endif
int64_t file_read(FD_TYPE fd, unsigned char *buf, uint64_t count);
int64_t file_write(FD_TYPE fd,unsigned char *buf, uint64_t count);
FD_TYPE open_file(const char * file,uint32_t flag);
int close_file(FD_TYPE fd);
/*
 * -1 文件存在
 * 0 文件不存在
 * 其他为错误号
 */
int check_file_exist(const char *filename);
#ifdef __cplusplus
}
#endif

#endif /* LIB_UTIL_FILE_OPT_H_ */
